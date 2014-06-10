#include <assimp/Importer.hpp>
#include <assimp/types.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <vector>
#include <map>

#include <Core/Engine.hh>
#include <Core/SceneManager.hh>
#include <Core/Renderer.hh>

// DEPENDENCIES
#include <Context/SdlContext.hh>
#include <Core/ConfigurationManager.hpp>
#include <Physic/BulletDynamicManager.hpp>
#include <Core/Timer.hh>
#include <Utils/PubSub.hpp>

namespace AGE
{
	struct Bone
	{
		std::string name;
		glm::mat4 offset;
		unsigned int index;
		glm::mat4 transformation;
		std::vector<unsigned int> children;
		unsigned int parent = (unsigned int)(-1);
	};

	struct Mesh
	{
		std::vector<glm::vec4> positions;
		std::vector<glm::vec4> normals;
		std::vector<glm::vec4> tangents;
		std::vector<glm::vec4> biTangents;
		std::vector<std::vector<glm::vec2>> uvs;
		std::vector<std::uint32_t> indices;
		std::vector<glm::vec4> weights;
		std::vector<glm::vec4> boneIndices;
		std::vector<std::string>bones;
	};
}

static glm::mat4 aiMat4ToGlm(const aiMatrix4x4 &m)
{
	return glm::mat4(m.a1, m.b1, m.c1, m.d1, m.a2, m.b2, m.c2, m.d2, m.a3, m.b3, m.c3, m.d3, m.a4, m.b4, m.c4, m.d4);
}


void readNodeHierarchy(unsigned int boneID, const glm::mat4 &parentTrans, std::vector<AGE::Bone> &bones, std::vector<glm::mat4> &trans)
{
	trans.resize(bones.size());

	glm::mat4 t = parentTrans * bones[boneID].transformation;
	trans[boneID] = t * bones[boneID].offset;

	for (unsigned int i = 0; i < bones[boneID].children.size(); ++i)
	{
		readNodeHierarchy(bones[boneID].children[i], t, bones, trans);
	}
}

int			main(int ac, char **av)
{


	std::shared_ptr<Engine>	e = std::make_shared<Engine>();

	// Set Configurations
	auto config = e->setInstance<ConfigurationManager>(File("MyConfigurationFile.conf"));

	e->setInstance<PubSub::Manager>();
	e->setInstance<SdlContext, IRenderContext>();
	e->setInstance<Input>();
	e->setInstance<Timer>();
	e->setInstance<Renderer>();
	e->setInstance<SceneManager>();
	e->setInstance<AssetsManager>()->init();

	// init engine
	if (e->init(0, 800, 600, "~AGE~ V0.0 Demo") == false)
		return (EXIT_FAILURE);

	// Set default window size
	// If config file has different value, it'll be changed automaticaly
	config->setConfiguration<glm::uvec2>("windowSize", glm::uvec2(800, 600), [&e](const glm::uvec2 &v)
	{
		e->getInstance<IRenderContext>()->setScreenSize(std::move(v));
	});

	config->loadFile();

	std::array<Attribute, 3> param =
	{
		Attribute(GL_FLOAT, sizeof(float), 4), //Positions
		Attribute(GL_FLOAT, sizeof(float), 4), //Weights
		Attribute(GL_FLOAT, sizeof(float), 4) //Bone indices
	};

	e->setInstance<VertexManager<3>>(param)->init();

	//if (!loadShaders(e))
	//	return EXIT_FAILURE;
	//if (!loadAssets(e))
	//	return EXIT_FAILURE;

	// launch engine
	if (e->start() == false)
		return (EXIT_FAILURE);

	Assimp::Importer importer;

	const aiScene *scene = importer.ReadFile("../../Assets/catwoman/atk close front 6.fbx"
		, aiProcess_Triangulate |
		aiProcess_CalcTangentSpace |
		/*aiProcess_JoinIdenticalVertices |*/
		aiProcess_SortByPType |
		aiProcess_ImproveCacheLocality |
		aiProcess_OptimizeMeshes);

	if (!scene)
	{
		std::cerr << "Scene load error" << std::endl;
		return EXIT_FAILURE;
	}

	std::vector<AGE::Mesh> meshs;
	std::vector<AGE::Bone> bones;
	unsigned int skeletonRoot = 0;

	meshs.resize(scene->mNumMeshes);

	// we reference all bones
	unsigned int boneCounter = 0;
	std::map<std::string, unsigned int> bonesTable;
	for (unsigned int meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex)
	{
		aiMesh *mesh = scene->mMeshes[meshIndex];

		for (unsigned int i = 0; i < mesh->mNumBones; ++i)
		{
			std::string boneName = mesh->mBones[i]->mName.data;
			if (bonesTable.find(boneName) != std::end(bonesTable))
				continue;
			if (meshs[meshIndex].bones.size() <= boneCounter)
				meshs[meshIndex].bones.resize(boneCounter + 1);
			meshs[meshIndex].bones[boneCounter] = boneName;
			bones.push_back(AGE::Bone());
			bones.back().index = boneCounter;
			bones.back().name = boneName;
			bones.back().offset = aiMat4ToGlm(mesh->mBones[i]->mOffsetMatrix);
			bonesTable.insert(std::make_pair(boneName, boneCounter));
			boneCounter++;
		}
	}

	for (unsigned int animIndex = 0; animIndex < scene->mNumAnimations; ++animIndex)
	{
		aiAnimation *animation = scene->mAnimations[animIndex];

		for (unsigned int i = 0; i < animation->mNumChannels; ++i)
		{
			auto channel = animation->mChannels[i];
			std::string boneName = channel->mNodeName.data;
			if (bonesTable.find(boneName) != std::end(bonesTable))
				continue;
			bones.push_back(AGE::Bone());
			bones.back().index = boneCounter;
			bones.back().name = boneName;
			bones.back().offset = glm::mat4(1);
			bonesTable.insert(std::make_pair(boneName, boneCounter));
			boneCounter++;
		}
	}

	//we fill bone hierarchy
	for (unsigned int i = 0; i < bones.size(); ++i)
	{
		aiNode *bonenode = scene->mRootNode->FindNode(aiString(bones[i].name));
		if (!bonenode)
			continue;

		//we set bone transformation
		bones[i].transformation = aiMat4ToGlm(bonenode->mTransformation);

		// we set parent
		if (bonenode->mParent != nullptr && bonesTable.find(bonenode->mParent->mName.data) == std::end(bonesTable))
		{
			auto parent = bonenode->mParent;
			while (parent && bonesTable.find(parent->mName.data) == std::end(bonesTable))
			{
				bones[i].transformation = aiMat4ToGlm(parent->mTransformation) * bones[i].transformation;
				parent = parent->mParent;
			}
			if (parent)
			{
				bones[i].parent = bonesTable.find(parent->mName.data)->second;				
			}
			else
			{
				skeletonRoot = i;
			}
		}
		else if (bonenode->mParent)
		{
			bones[i].parent = bonesTable.find(bonenode->mParent->mName.data)->second;
		}

		//we set children
		for (unsigned int c = 0; c < bonenode->mNumChildren; ++c)
		{
			auto f = bonesTable.find(bonenode->mChildren[c]->mName.data);
			if (f == std::end(bonesTable))
				continue;
			bones[i].children.push_back(f->second);
		}
	}


	for (unsigned int meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex)
	{
		aiMesh *mesh = scene->mMeshes[meshIndex];
		std::uint32_t indice = 0;

		unsigned int meshFacesNbr = mesh->mNumFaces;
		for (unsigned int faceIndex = 0; faceIndex < meshFacesNbr; ++faceIndex)
		{
			const aiFace &face = mesh->mFaces[faceIndex];
			if (mesh->HasPositions())
			{
				for (unsigned int k = 0; k < 3; ++k)
				{
					auto &aiPositions = mesh->mVertices[face.mIndices[k]];
					meshs[meshIndex].positions.push_back(glm::vec4(aiPositions.x, aiPositions.y, aiPositions.z, 1));
				}
			}
			if (mesh->HasNormals())
			{
				for (unsigned int k = 0; k < 3; ++k)
				{
					auto &aiNormals = mesh->mNormals[face.mIndices[k]];
					meshs[meshIndex].normals.push_back(glm::vec4(aiNormals.x, aiNormals.y, aiNormals.z, 1));
				}
			}
			for (unsigned int texCoordIndex = 0; texCoordIndex < AI_MAX_NUMBER_OF_TEXTURECOORDS; ++texCoordIndex)
			{
				if (mesh->HasTextureCoords(texCoordIndex))
				{
					meshs[meshIndex].uvs.resize(texCoordIndex + 1);
					for (unsigned int k = 0; k < 3; ++k)
					{
						auto &aiUvs = mesh->mTextureCoords[texCoordIndex][face.mIndices[k]];
						meshs[meshIndex].uvs[texCoordIndex].push_back(glm::vec2(aiUvs.x, aiUvs.y));
					}
				}
			}
			if (mesh->HasTangentsAndBitangents())
			{
				for (unsigned int k = 0; k < 3; ++k)
				{
					auto &aiTangents = mesh->mTangents[face.mIndices[k]];
					meshs[meshIndex].tangents.push_back(glm::vec4(aiTangents.x, aiTangents.y, aiTangents.z, 1));
					auto &aiBiTangents = mesh->mBitangents[face.mIndices[k]];
					meshs[meshIndex].biTangents.push_back(glm::vec4(aiBiTangents.x, aiBiTangents.y, aiBiTangents.z, 1));
				}
			}

			for (unsigned int i = 0; i < 3; ++i)
			{
				meshs[meshIndex].indices.push_back(indice);
				++indice;
			}
		}

		std::map<std::string, unsigned int> bonesIndices;
		unsigned int numBone = 0;

		meshs[meshIndex].weights.resize(meshs[meshIndex].positions.size(), glm::vec4(0));
		meshs[meshIndex].boneIndices.resize(meshs[meshIndex].positions.size(), glm::vec4(0));

		for (unsigned int i = 0; i < mesh->mNumBones; ++i)
		{
			unsigned int boneIndex = bonesTable.find(mesh->mBones[i]->mName.data)->second;

			for (unsigned int j = 0; j < mesh->mBones[i]->mNumWeights; ++j)
			{
				float weight = mesh->mBones[i]->mWeights[j].mWeight;
				float vid = mesh->mBones[i]->mWeights[j].mVertexId;
				if (meshs[meshIndex].weights[vid].r == 0.0f)
				{
					meshs[meshIndex].weights[vid].r = weight;
					meshs[meshIndex].boneIndices[vid].r = boneIndex;
				}
				else if (meshs[meshIndex].weights[vid].g == 0.0f)
				{
					meshs[meshIndex].weights[vid].g = weight;
					meshs[meshIndex].boneIndices[vid].g = boneIndex;
				}
				else if (meshs[meshIndex].weights[vid].b == 0.0f)
				{
					meshs[meshIndex].weights[vid].b = weight;
					meshs[meshIndex].boneIndices[vid].b = boneIndex;
				}
				else if (meshs[meshIndex].weights[vid].a == 0.0f)
				{
					meshs[meshIndex].weights[vid].a = weight;
					meshs[meshIndex].boneIndices[vid].a = boneIndex;
				}
			}
		}
	}

	auto shader = e->getInstance<Renderer>()->addShader("basic",
		"./basic.vp",
		"./basic.fp");
	if (shader->getId() < 0)
		return EXIT_FAILURE;
	shader->use();

	std::vector<Vertice<3>*> vertices;
	vertices.resize(meshs.size());

	for (unsigned int i = 0; i < meshs.size(); ++i)
	{
		std::array<Data, 3> data =
		{
			Data(meshs[i].positions.size() * 4 * sizeof(float), &meshs[i].positions[0].x)
			, Data(meshs[i].weights.size() * 4 * sizeof(float), &meshs[i].weights[0].x)
			, Data(meshs[i].boneIndices.size() * 4 * sizeof(float), &meshs[i].boneIndices[0].x)
		};

		Data *indicesData = new Data(meshs[i].indices.size() * sizeof(unsigned int), &meshs[i].indices[0]);
		vertices[i] = new Vertice<3>(meshs[i].positions.size(), data, indicesData);
		e->getInstance<VertexManager<3>>()->addVertice(*vertices[i]);
	}


	// Projection matrix : 45� Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	glm::mat4 Projection = glm::perspective(60.0f, 16.0f / 9.0f, 0.1f, 1000.0f);
	// Camera matrix
	float lookAtX = 0;
	float lookAtY = 0;
	// Model matrix : an identity matrix (model will be at the origin)
	glm::mat4 Model = glm::mat4(1.0f);  // Changes for each model !
	Model = glm::scale(Model, glm::vec3(0.3f));


	// On enable la depth car par defaut elle est pas active
	// Conseil : la mettre dans son game engine
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	// la depth de clear par defaut sera 1
	glClearDepth(1.0f);
	// la couleur de clear par defaut sera du noir
	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

	do
	{
		auto time = e->getInstance<Timer>()->getElapsed();
		glm::vec4 color;
		shader->use();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		Model = glm::rotate(Model, 20.0f * (float)time, glm::vec3(0, 1, 0));
		color = glm::vec4(1, 0, 1, 1);

		if (e->getInstance<Input>()->getKey(SDLK_w))
			lookAtY += 1;
		if (e->getInstance<Input>()->getKey(SDLK_s))
			lookAtY -= 1;
		if (e->getInstance<Input>()->getKey(SDLK_a))
			lookAtX -= 1;
		if (e->getInstance<Input>()->getKey(SDLK_d))
			lookAtX += 1;

		glm::mat4 View = glm::lookAt(
		glm::vec3(lookAtX, lookAtY, 150), // Camera is at (4,3,3), in World Space
		glm::vec3(lookAtX, lookAtY, 0), // and looks at the origin
		glm::vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
		);	

		auto colorId = glGetUniformLocation(shader->getId(), "color");
		auto modelId = glGetUniformLocation(shader->getId(), "model");
		auto viewId = glGetUniformLocation(shader->getId(), "view");
		auto projectionId = glGetUniformLocation(shader->getId(), "projection");

		glUniform4fv(colorId, 1, &color[0]);
		glUniformMatrix4fv(modelId, 1, GL_FALSE, &Model[0][0]);
		glUniformMatrix4fv(viewId, 1, GL_FALSE, &View[0][0]);
		glUniformMatrix4fv(projectionId, 1, GL_FALSE, &Projection[0][0]);

		std::vector<glm::mat4> bonesTrans;
		bonesTrans.resize(bones.size());
		readNodeHierarchy(skeletonRoot, glm::mat4(1), bones, bonesTrans);
		glUniformMatrix4fv(glGetUniformLocation(shader->getId(), "bones"), bonesTrans.size(), GL_FALSE, glm::value_ptr(bonesTrans[0]));

		for (unsigned int i = 0; i < vertices.size(); ++i)
		{
			vertices[i]->draw(GL_TRIANGLES);
		}
	} while (e->update());

	config->saveToFile();
	e->stop();

	return EXIT_SUCCESS;
}