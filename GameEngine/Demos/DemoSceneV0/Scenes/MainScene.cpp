#include "MainScene.hh"

#include <glm/gtx/matrix_operation.hpp>

// components
#include <Components/FPController.hpp>
#include <Components/CameraComponent.hpp>
#include <Components/FirstPersonView.hpp>

//systems
#include <SharedSystems/FPSSystem.hh>
#include <Systems/FPControllerSystem.hpp>
#include <Systems/FirstPersonViewSystem.hpp>
#include <Systems/CameraSystem.hpp>
#include <Systems/LightRenderingSystem.hh>
#include <Systems/BulletDynamicSystem.hpp>
#include <Systems/SpriteSystem.hh>

// SDL
#include <Context/SdlContext.hh>

MainScene::MainScene(std::weak_ptr<Engine> engine)
: AScene(engine)
{}

MainScene::~MainScene(void)
{}


bool 			MainScene::userStart()
{
	//load shaders
	if (!loadShaders())
		return false;
	if (!loadAssets())
		return false;

	// add systems
	addSystem<FPControllerSystem>(10);
	addSystem<FirstPersonViewSystem>(20);
	addSystem<CameraSystem>(30);
	addSystem<BulletDynamicSystem>(35);
	addSystem<FPSSystem>(40);
	addSystem<LightRenderingSystem>(1000); // Render with the lights
	addSystem<SpriteSystem>(2000);

	getSystem<LightRenderingSystem>()->setHDRIdealIllumination(0.3f);
	getSystem<LightRenderingSystem>()->setHDRAdaptationSpeed(0.1f);

	getSystem<LightRenderingSystem>()->setHDRMaxLightDiminution(0.1f);
	getSystem<LightRenderingSystem>()->setHDRMaxDarkImprovement(1.2f);
	getSystem<LightRenderingSystem>()->useHDR(false);

	// create heros
	{
		_heros = createEntity();

		_heros->setLocalTransform(glm::translate(_heros->getLocalTransform(), glm::vec3(-8, 2, 0)));

		auto fpc = _heros->addComponent<Component::FPController>();
		fpc->getShape().setLocalScaling(btVector3(0.3f, 0.3f, 0.3f));
		auto camera = _heros->addComponent<Component::CameraComponent>();
		auto screenSize = getInstance<IRenderContext>()->getScreenSize();
		camera->viewport = glm::uvec4(0, 0, screenSize.x, screenSize.y);
		camera->attachSkybox("skybox__space", "cubemapShader");
		auto fpv = _heros->addComponent<Component::FirstPersonView>();
		auto l = _heros->addComponent<Component::PointLight>();
		l->lightData.colorRange = glm::vec4(1.0f,1.0f,1.0f, 50.0f);
		l->lightData.positionPower.w = 3.0f;
		_heros->addComponent<Component::Sprite>(getInstance<SpriteManager>()->getAnimation("Pong", "pong"),
			getInstance<Renderer>()->getShader("SpriteBasic"));
	}

	// create Entrance room
	{
		_entrance = std::make_unique<Entrance>(
			std::dynamic_pointer_cast<AScene>(shared_from_this())
			);
		if (!_entrance->init())
			return false;
		_entrance->enable();
	}

	return true;
}


bool 			MainScene::userUpdate(double time)
{
	float ftime = (float)(time);
	static float delay = 0.0f;
	if (getInstance<Input>()->getInput(SDL_BUTTON_LEFT) && delay <= 0.0f)
	{
		glm::vec3 from, to;
		getSystem<CameraSystem>()->getRayFromCenterOfScreen(from, to);
		auto e = createEntity();
		e->setLocalTransform(glm::translate(e->getLocalTransform(), glm::vec3(from + to * 1.5f)));
		e->setLocalTransform(glm::scale(e->getLocalTransform(), glm::vec3(0.3f)));
//		e->setLocalTransform(glm::lookAt(from, to, glm::vec3(0,1,0)));
		auto mesh = e->addComponent<Component::MeshRenderer>(getInstance<AssetsManager>()->get<ObjFile>("obj__cube"));
		mesh->setShader("MaterialBasic");
		auto rigidBody = e->addComponent<Component::RigidBody>(0.05f);
		rigidBody->setCollisionShape(Component::RigidBody::BOX);
		//auto l = e->addComponent<Component::PointLight>();
		//l->lightData.colorRange = glm::vec4(rand() % 10000 / 10000.0f, rand() % 10000 / 10000.0f, rand() % 10000 / 10000.0f, 5.0f);
		//l->lightData.positionPower.w = 3.0f;
		delay = 0.1f;
	}
	if (delay >= 0.0f)
		delay -= ftime;
	if (getInstance<Input>()->getInput(SDLK_ESCAPE) ||
		getInstance<Input>()->getInput(SDL_QUIT))
	{
		return false;
	}
	return true;
}

bool MainScene::loadShaders()
{
	std::string		perModelVars[] =
	{
		"model"
	};

	std::string	perFrameVars[] =
	{
		"projection",
		"view",
		"time",
		"pointLightNbr",
		"spotLightNbr"
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

	std::string	perLightVars[] =
	{
		"lightVP"
	};

	auto s = getInstance<Renderer>()->addShader("MaterialBasic",
		"../../Shaders/MaterialBasic.vp",
		"../../Shaders/MaterialBasic.fp");

	auto shadowDepth = getInstance<Renderer>()->addShader("ShadowDepth" , "../../Shaders/ShadowMapping.vp", "../../Shaders/ShadowMapping.fp");

	getInstance<Renderer>()->addUniform("MaterialBasic")
		->init(s, "MaterialBasic", materialBasic);
	getInstance<Renderer>()->addUniform("PerFrame")
		->init(s, "PerFrame", perFrameVars);
	getInstance<Renderer>()->addUniform("PerModel")
		->init(s, "PerModel", perModelVars);
	getInstance<Renderer>()->addUniform("PerLight")
		->init(shadowDepth, "PerLight", perLightVars);

	getInstance<Renderer>()->addShader("2DText",
		"../../Shaders/2DText.vp",
		"../../Shaders/2DText.fp");

	getInstance<Renderer>()->addShader("SpriteBasic",
		"../../Shaders/SpriteBasic.vp",
		"../../Shaders/SpriteBasic.fp");


	getInstance<Renderer>()->addShader("basicLight", "../../Shaders/light.vp", "../../Shaders/light.fp");
	getInstance<Renderer>()->addShader("depthOnly", "../../Shaders/depthOnly.vp", "../../Shaders/depthOnly.fp");
	getInstance<Renderer>()->bindShaderToUniform("ShadowDepth", "PerModel", "PerModel");
	getInstance<Renderer>()->bindShaderToUniform("ShadowDepth", "PerLight", "PerLight");

	getInstance<Renderer>()->bindShaderToUniform("depthOnly", "PerFrame", "PerFrame");
	getInstance<Renderer>()->bindShaderToUniform("depthOnly", "PerModel", "PerModel");

	getInstance<Renderer>()->bindShaderToUniform("MaterialBasic", "PerFrame", "PerFrame");
	getInstance<Renderer>()->bindShaderToUniform("MaterialBasic", "PerModel", "PerModel");
	getInstance<Renderer>()->bindShaderToUniform("MaterialBasic", "MaterialBasic", "MaterialBasic");

	std::string	vars[] =
	{
		"projection",
		"view"
	};

	auto sky = getInstance<Renderer>()->addShader("cubemapShader", "../../Shaders/cubemap.vp", "../../Shaders/cubemap.fp");

	getInstance<Renderer>()->getShader("cubemapShader")->addTarget(GL_COLOR_ATTACHMENT0).setTextureNumber(1).build();

	getInstance<Renderer>()->addUniform("cameraUniform")
		->init(sky, "cameraUniform", vars);

	getInstance<Renderer>()->bindShaderToUniform("cubemapShader", "cameraUniform", "cameraUniform");

	return true;
}

bool MainScene::loadAssets()
{
	getInstance<AssetsManager>()->loadFromList(File("../../Assets/Serialized/export__Space.cpd"));
	getInstance<AssetsManager>()->loadFromList(File("../../Assets/Serialized/export__cube.cpd"));
	getInstance<SpriteManager>()->loadFile(File("../../Assets/Serialized/Pong.CPDAnimation"));
	return true;
}