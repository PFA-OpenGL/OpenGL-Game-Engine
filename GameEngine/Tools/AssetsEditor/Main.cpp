#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/matrix_interpolation.hpp>

#include <vector>
#include <map>
#include <chrono>

#include <Core/Engine.hh>
#include <Core/SceneManager.hh>
#include <Core/Renderer.hh>

// DEPENDENCIES
#include <Context/SdlContext.hh>
#include <Core/ConfigurationManager.hpp>
#include <Physic/BulletDynamicManager.hpp>
#include <Core/Timer.hh>
#include <Utils/PubSub.hpp>
#include <OpenGL/GeometryManager.hh>
#include <Utils/PerformanceDebugger.hh>
#include <Geometry/MeshManager.hpp>

#include <Convertor/SkeletonLoader.hpp>
#include <Convertor/AnimationsLoader.hpp>
#include <Convertor/MeshLoader.hpp>

#include <Geometry/MeshManager.hpp>

//SKINNING
#include <Skinning/Animation.hpp>
#include <Skinning/AnimationChannel.hpp>
#include <Skinning/AnimationInstance.hpp>
#include <Skinning/AnimationKey.hpp>
#include <Skinning/Bone.hpp>
#include <Skinning/Skeleton.hpp>


int			main(void)
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
	e->setInstance<PerformanceDebugger>();
	e->setInstance<AGE::MeshManager>();
	auto geometryManager = e->setInstance<gl::GeometryManager>();

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

	// launch engine
	if (e->start() == false)
		return (EXIT_FAILURE);

	//////////////////////////
	/////
	//

	//Convert fbx to AGE data structure
   	AGE::AssetDataSet dataSet;
	dataSet.filePath = File("../../Assets/catwoman/atk close front 6.fbx");
	dataSet.name = "catwoman";
	auto isSkeleton = AGE::SkeletonLoader::load(dataSet);
	auto isAnimations = AGE::AnimationsLoader::load(dataSet);
	auto isMesh = AGE::MeshLoader::load(dataSet);
	
	//Save AGE assets data structure to filesystem

	//Load assets from serialized file
	auto meshManager = e->getInstance<AGE::MeshManager>();

	//meshManager->load("catwoman.sage"); // load mesh
	//meshManager->load("roulade.aage"); // load animation
	//meshManager->load("catwoman.skage"); // load skeleton

	//
	/////
	//////////////////////////



	auto shader = e->getInstance<Renderer>()->addShader("basic",
		"./basic.vp",
		"./basic.fp");
	if (shader->getId() < 0)
		return EXIT_FAILURE;
	shader->use();



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

	std::array<AGE::AnimationInstance*, 100> animationInstances;
	//animationInstances.fill(new AGE::AnimationInstance(&skeleton, &animations[0]));

	do
	{
		auto time = e->getInstance<Timer>()->getElapsed();
		static float totalTime = 0.0f;
		totalTime += time;

		glm::vec4 color;
		shader->use();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
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

		e->getInstance<PerformanceDebugger>()->start("Skinning");
		for (auto &&a : animationInstances)
		{
			a->update(totalTime * 10.0f);
		}
		//skeleton.updateSkinning();
		static float median = 0.0f;
		glUniformMatrix4fv(glGetUniformLocation(shader->getId(), "bones"), animationInstances[0]->transformations.size(), GL_FALSE, glm::value_ptr(animationInstances[0]->transformations[0]));

		//for (unsigned int i = 0; i < vertices.size(); ++i)
		{
			//geometryManager->draw(GL_TRIANGLES, indices[i], vertices[i]);
		}
	} while (e->update());

	config->saveToFile();
	e->stop();
	return EXIT_SUCCESS;
}