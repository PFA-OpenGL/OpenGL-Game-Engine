#include "WorldEditorScene.hpp"
#include <imgui\imgui.h>
#include <Systems/EntityManager.hpp>
#include <Systems/AssetsAndComponentRelationsSystem.hpp>
#include <Physic/BulletDynamicManager.hpp>
#include <AssetManagement/AssetManager.hh>

#include <Components/CameraComponent.hpp>

#include <Threads/ThreadManager.hpp>
#include <Threads/RenderThread.hpp>
#include <Threads/PrepareRenderThread.hpp>
#include <Threads/Commands/MainToPrepareCommands.hpp>
#include <Threads/Commands/ToRenderCommands.hpp>
#include <Threads/Tasks/BasicTasks.hpp>
#include <Threads/Tasks/ToRenderTasks.hpp>

namespace AGE
{
	const std::string WorldEditorScene::Name = "WorldEditor";

	WorldEditorScene::WorldEditorScene(AGE::Engine *engine)
		: AScene(engine)
	{
	}

	WorldEditorScene::~WorldEditorScene(void)
	{
	}

	bool WorldEditorScene::userStart()
	{
		setInstance<AGE::BulletDynamicManager, AGE::BulletCollisionManager>()->init();
		setInstance<AssetsManager>();
		getInstance<AGE::AssetsManager>()->setAssetsDirectory("../../Assets/Serialized/");


		addSystem<WE::AssetsAndComponentRelationsSystem>(0);
		addSystem<WE::EntityManager>(1);

		auto camera = createEntity();
		auto cam = camera.addComponent<CameraComponent>();

		return true;
	}

	bool WorldEditorScene::userUpdateBegin(double time)
	{
		ImGui::BeginChild("Assets browser", ImVec2(0, 0), true);
		return true;
	}

	bool WorldEditorScene::userUpdateEnd(double time)
	{
		ImGui::EndChild();
		ImGui::End();

		// TODO
		AGE::GetPrepareThread()->getQueue()->emplaceCommand<AGE::Commands::MainToPrepare::PrepareDrawLists>();
		// TODO
		AGE::GetPrepareThread()->getQueue()->emplaceCommand<AGE::Commands::ToRender::RenderDrawLists>();

		return true;
	}
}