#ifndef		__INGAME_SCENE_HPP__
#define		__INGAME_SCENE_HPP__

#include <Core/Engine.hh>
#include <Core/AScene.hh>

/////////////
// SYSTEMS
/////////////

#include <Systems/LightRenderingSystem.hh>
#include <Systems/TrackBallSystem.hpp>
#include <Systems/CameraSystem.hpp>
#include <Systems/BulletCollisionSystem.hpp>
#include <Systems/VelocitySystem.hpp>
#include <Systems/CollisionAdderSystem.hpp>
#include <Systems/CollisionCleanerSystem.hpp>
#include <Systems/BulletDynamicSystem.hpp>
#include <Systems/SpaceshipControllerSystem.hpp>
#include <Systems/FirstPersonViewSystem.hpp>
#include <Systems/TrackingCameraSystem.hpp>

////////////
// COMPONENTS
////////////
#include <Components/CameraComponent.hpp>
#include <Components/FPController.hpp>
#include <Components/FirstPersonView.hpp>
#include <Components/VelocityComponent.hpp>
#include <Components/RigidBody.hpp>
#include <Components/SpaceshipController.hpp>

#include <MediaFiles/AssetsManager.hpp>

class InGameScene : public AScene
{
	Entity test;
public:
	InGameScene(Engine &engine)
		: AScene(engine)
	{}

	virtual ~InGameScene(void)
	{}

	virtual bool 			userStart()
	{
		addSystem<BulletDynamicSystem>(10); // CHECK FOR COLLISIONS
		addSystem<CollisionAdder>(20); // ADD COLLISION COMPONENT TO COLLIDING ELEMENTS
		addSystem<VelocitySystem>(50); // UPDATE VELOCITY
		addSystem<SpaceshipControllerSystem>(60); // UPDATE FIRST PERSON CONTROLLER
		addSystem<TrackingCameraSystem>(150); // UPDATE CAMERA TRACKING BEHAVIOR
		addSystem<FirstPersonViewSystem>(150); // UPDATE FIRST PERSON CAMERA
		addSystem<CameraSystem>(200); // UPDATE CAMERA AND RENDER TO SCREEN
		addSystem<LightRenderingSystem>(250);
		addSystem<CollisionCleaner>(300); // REMOVE COLLISION COMPONENT FROM COLLIDING ELEMENTS

		std::string		perModelVars[] =
		{
			"model"
		};

		std::string		perFrameVars[] =
		{
			"projection",
			"view",
			"time"
		};

		std::string		materialBasic[] =
		{
			"ambient",
			"diffuse",
			"specular",
			"transmittance",
			"emission",
			"shininess"
		};


		OpenGLTools::Shader &s = _engine.getInstance<Renderer>()->addShader("MaterialBasic",
			"./Shaders/MaterialBasic.vp",
			"./Shaders/MaterialBasic.fp");

		_engine.getInstance<Renderer>()->addUniform("MaterialBasic")
			.init(&s, "MaterialBasic", materialBasic);
		_engine.getInstance<Renderer>()->addUniform("PerFrame")
			.init(&s, "PerFrame", perFrameVars);
		_engine.getInstance<Renderer>()->addUniform("PerModel")
			.init(&s, "PerModel", perModelVars);


		_engine.getInstance<Renderer>()->addShader("fboToScreen", "Shaders/fboToScreen.vp", "Shaders/fboToScreen.fp");

		_engine.getInstance<Renderer>()->bindShaderToUniform("MaterialBasic", "PerFrame", "PerFrame");
		_engine.getInstance<Renderer>()->bindShaderToUniform("MaterialBasic", "PerModel", "PerModel");
		_engine.getInstance<Renderer>()->bindShaderToUniform("MaterialBasic", "MaterialBasic", "MaterialBasic");

		getInstance<AssetsManager>()->loadFromList(File("./Assets/Serialized/export__cube.cpd"));
		getInstance<AssetsManager>()->loadFromList(File("./Assets/Serialized/export__ball.cpd"));
		getInstance<AssetsManager>()->loadFromList(File("./Assets/Serialized/export__Space.cpd"));
		getInstance<AssetsManager>()->loadFromList(File("./Assets/Serialized/export__galileo.cpd"));
		getInstance<AssetsManager>()->loadFromList(File("./Assets/Serialized/export__sponza.cpd"));

		std::string		vars[] =
		{
			"projection",
			"view"
		};

		OpenGLTools::Shader &sky = _engine.getInstance<Renderer>()->addShader("cubemapShader", "Shaders/cubemap.vp", "Shaders/cubemap.fp");
		_engine.getInstance<Renderer>()->getShader("cubemapShader")->addTarget(GL_COLOR_ATTACHMENT0).setTextureNumber(1).build();
		_engine.getInstance<Renderer>()->addUniform("cameraUniform").
			init(&sky, "cameraUniform", vars);
		_engine.getInstance<Renderer>()->bindShaderToUniform("cubemapShader", "cameraUniform", "cameraUniform");


		/////////////////////////////
		/////
		/////
		/////   !!!! GAME
		/////
		////
		///
		//

		// HEROS
		Entity heros;
		{
			Entity e = createEntity();
			auto t = e->getLocalTransform();
			t = glm::translate(t, glm::vec3(0, 0, 0));
			t = glm::scale(t, glm::vec3(2));
			e->setLocalTransform(t);
			auto rigidBody = e->addComponent<Component::RigidBody>();
			rigidBody->setMass(0.0f);
			rigidBody->setCollisionShape(Component::RigidBody::BOX, "obj__galileo");
			auto mesh = e->addComponent<Component::MeshRenderer>(getInstance<AssetsManager>()->get<ObjFile>("obj__galileo"));
			mesh->setShader("MaterialBasic");
			e->addComponent<Component::SpaceshipController>();
			heros = e;
		}

		Entity floor;
		{
			Entity e = createEntity();
			auto t = e->getLocalTransform();
			t = glm::translate(t, glm::vec3(0, -10, 0));
			t = glm::scale(t, glm::vec3(100, 100, 100));
			e->setLocalTransform(t);
			auto rigidBody = e->addComponent<Component::RigidBody>();
			rigidBody->setMass(0.0f);
			//rigidBody->setCollisionShape(Component::RigidBody::BOX);
			rigidBody->setCollisionShape(Component::RigidBody::MESH, "collision_shape_static_sponza");
			auto mesh = e->addComponent<Component::MeshRenderer>(getInstance<AssetsManager>()->get<ObjFile>("obj__sponza"));
			mesh->setShader("MaterialBasic");
			floor = e;
		}


		{
			Entity e = createEntity();
			auto t = e->getLocalTransform();
			t = glm::translate(t, glm::vec3(50, -10, 50));
			t = glm::scale(t, glm::vec3(50,50,50));
			e->setLocalTransform(t);
			auto rigidBody = e->addComponent<Component::RigidBody>();
			rigidBody->setMass(0.0f);
			rigidBody->setCollisionShape(Component::RigidBody::MESH, "collision_shape_static_sponza");
			auto mesh = e->addComponent<Component::MeshRenderer>(getInstance<AssetsManager>()->get<ObjFile>("obj__sponza"));
			mesh->setShader("MaterialBasic");
			test = e;
		}

		auto camera = createEntity();
		auto cameraComponent = camera->addComponent<Component::CameraComponent>();
		auto trackBall = camera->addComponent<Component::TrackingCamera>(heros, glm::vec3(0, 0, -5.0f));
		cameraComponent->attachSkybox("skybox__space", "cubemapShader");
		return true;
	}

	Entity                  createHeros(const glm::vec3 &pos)
	{
		auto e = createEntity();
		e->setLocalTransform(glm::translate(e->getLocalTransform(), pos));
		return e;
	}

	virtual bool 			userUpdate(double time)
	{
		static std::vector<Entity> balls;

		if (_engine.getInstance<Input>()->getInput(SDLK_ESCAPE) ||
			_engine.getInstance<Input>()->getInput(SDL_QUIT))
			return (false);
		static auto timer = 0.0f;

		if (_engine.getInstance<Input>()->getInput(SDLK_d))
		{
			for (auto &e : balls)
				destroy(e);
			balls.clear();
		}

		if (_engine.getInstance<Input>()->getInput(SDLK_u))
		{
			test->setLocalTransform(glm::scale(test->getLocalTransform(), glm::vec3(0.9)));
		}

		if (_engine.getInstance<Input>()->getInput(SDLK_i))
		{
			test->setLocalTransform(glm::scale(test->getLocalTransform(), glm::vec3(1.1)));
		}

		if (_engine.getInstance<Input>()->getInput(SDLK_r) && timer <= 0.0f)
		{
			timer = 0.3f;
			for (auto i = 0; i < 10; ++i)
			{
				auto e = createHeros(glm::vec3(rand() % 20, 50 + rand() % 100, rand() % 20));
				e->setLocalTransform(glm::scale(e->getLocalTransform(), glm::vec3((float)(rand() % 10) / 0.8)));
				auto rigidBody = e->addComponent<Component::RigidBody>();
				rigidBody->setMass(1.0f);
				rigidBody->setCollisionShape(Component::RigidBody::MESH, "collision_shape_dynamic_galileo");
				auto mesh = e->addComponent<Component::MeshRenderer>(getInstance<AssetsManager>()->get<ObjFile>("obj__galileo"));
				mesh->setShader("MaterialBasic");
				balls.push_back(e);
			}
			std::cout << balls.size() << std::endl;
		}
		if (timer > 0.0f)
			timer -= time;		
		return (true);
	}
};

#endif //__INGAME_SCENE_HPP__