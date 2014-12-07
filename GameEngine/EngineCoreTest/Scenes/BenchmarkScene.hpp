#pragma once

#include <Core/AScene.hh>
#include <Systems/LifetimeSystem.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <Systems/BulletDynamicSystem.hpp>
#include <Systems/CollisionAdderSystem.hpp>
#include <Systems/CollisionCleanerSystem.hpp>
#include <Systems/CameraSystem.hh>
#include <Components/MeshRenderer.hh>
#include <Components/Light.hh>
#include <Core/AssetsManager.hpp>
#include <Core/PrepareRenderThread.hpp>
#include <Core/PrepareRenderThreadCommand.hpp>
#include <Utils/ThreadQueueCommands.hpp>
#include <Context/IRenderContext.hh>
#include <Core/RenderThread.hpp>
#include <CONFIGS.hh>
#include <Skinning/AnimationInstance.hpp>

//for test
#include <Utils/Containers/CommandQueue.hpp>

# define VERTEX_SHADER "../../Shaders/test_pipeline_1.vp"
# define FRAG_SHADER "../../Shaders/test_pipeline_1.fp"
# define DEFFERED_VERTEX_SHADER "../../Shaders/Deffered_shading/deffered_shading_get_buffer.vp"
# define DEFFERED_FRAG_SHADER "../../Shaders/Deffered_shading/deffered_shading_get_buffer.fp"
# define DEFFERED_VERTEX_SHADER_ACCUM "../../Shaders/Deffered_shading/deffered_shading_lighting.vp"
# define DEFFERED_FRAG_SHADER_ACCUM "../../Shaders/Deffered_shading/deffered_shading_lighting.fp"
# define DEFERRED_FRAG_SHADER_MERGE "../../Shaders/Deffered_shading/deferred_shading_merge.fp"
# define DEFERRED_VERTEX_SHADER_MERGE "../../Shaders/Deffered_shading/deferred_shading_merge.vp"

class BenchmarkScene : public AScene	
{
public:
	BenchmarkScene(std::weak_ptr<Engine> &&engine);

	virtual ~BenchmarkScene(void);
	void initRendering();
	virtual bool userStart();
	virtual bool userUpdate(double time);

private:
	AGE::RenderThread *_renderThread;

	std::size_t _frameCounter = 0;
	double _timeCounter = 0.0;
	double _maxTime = 10000000005.0f;
	double _chunkCounter = 0.0;
	double _maxChunk = 0.25f;
	std::size_t _chunkFrame = 0;
	std::ofstream _logFile;
	Entity GLOBAL_CAMERA;
	Entity GLOBAL_CATWOMAN;
	Entity GLOBAL_LIGHT;
	Entity GLOBAL_SPONZA;
	Entity GLOBAL_FLOOR;
	Entity GLOBAL_HEAD;
	Key<AGE::AnimationInstance> GLOBAL_CAT_ANIMATION;
};