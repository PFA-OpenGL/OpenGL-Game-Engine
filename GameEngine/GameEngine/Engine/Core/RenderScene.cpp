#include "RenderScene.hpp"
#include <Threads/PrepareRenderThread.hpp>
#include <Threads/RenderThread.hpp>
#include <Threads/ThreadManager.hpp>
#include <Core/OctreeNode.hh>
#include <Core/Commands/Render.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <chrono>

namespace AGE
{
	RenderScene::RenderScene(PrepareRenderThread *prepareThread, Engine *engine, AScene *scene)
		: _prepareThread(prepareThread)
		, _engine(engine)
		, _scene(scene)
		, _MeshCounter(0)
		, _pointLightCounter(0)
		, _cameraCounter(0)
	{
		_drawables.reserve(65536);
		_octree = new OctreeNode()			;
		assert(prepareThread && engine && scene);
	}

	RenderScene::~RenderScene(void)
	{
		if (_octree)
			delete _octree;
	}

	bool RenderScene::init()
	{
		return true;
	}

	PrepareKey RenderScene::addMesh()
	{
		PrepareKey res;
		res.type = PrepareKey::Type::Drawable;
		if (!_freeMeshs.empty())
		{
			res.id = _freeMeshs.front();
			_freeMeshs.pop();
		}
		else
			res.id = PrepareKey::OctreeObjectId(_MeshCounter++);
		_prepareThread->getQueue()->emplaceCommand<Commands::MainToPrepare::CreateDrawable>(res);
		return res;
	}

	PrepareKey RenderScene::addCamera()
	{
		PrepareKey res;
		res.type = PrepareKey::Type::Camera;
		if (!_freeCameras.empty())
		{
			res.id = _freeCameras.front();
			_freeCameras.pop();
		}
		else
			res.id = PrepareKey::OctreeObjectId(_cameraCounter++);
		_prepareThread->getQueue()->emplaceCommand<Commands::MainToPrepare::CreateCamera>(res);
		return res;
	}

	PrepareKey RenderScene::addPointLight()
	{
		PrepareKey res;
		res.type = PrepareKey::Type::PointLight;
		if (!_freePointLights.empty())
		{
			res.id = _freePointLights.front();
			_freePointLights.pop();
		}
		else
			res.id = PrepareKey::OctreeObjectId(_pointLightCounter++);
		_prepareThread->getQueue()->emplaceCommand<Commands::MainToPrepare::CreatePointLight>(res);
		return res;
	}

	RenderScene &RenderScene::removeElement(const PrepareKey &key)
	{
		assert(!key.invalid());
		switch (key.type)
		{
		case PrepareKey::Type::Camera:
			_freeCameras.push(key.id);
			_prepareThread->getQueue()->emplaceCommand<Commands::MainToPrepare::DeleteCamera>(key);
			break;
		case PrepareKey::Type::Drawable:
			_freeMeshs.push(key.id);
			_prepareThread->getQueue()->emplaceCommand<Commands::MainToPrepare::DeleteDrawable>(key);
			break;
		case PrepareKey::Type::PointLight:
			_freePointLights.push(key.id);
			_prepareThread->getQueue()->emplaceCommand<Commands::MainToPrepare::DeletePointLight>(key);
			break;
		default:
			break;
		}
		return (*this);
	}

	RenderScene &RenderScene::setPointLight(glm::vec3 const &color, glm::vec3 const &range, const PrepareKey &id)
	{
		_prepareThread->getQueue()->emplaceCommand<Commands::MainToPrepare::SetPointLight>(color, range, id);
		return (*this);
	}

	RenderScene &RenderScene::setPosition(const glm::vec3 &v, const PrepareKey &id)
	{
		_prepareThread->getQueue()->emplaceCommand<Commands::MainToPrepare::SetPosition>(id, v);
		return (*this);
	}

	RenderScene &RenderScene::setOrientation(const glm::quat &v, const PrepareKey &id)
	{
		_prepareThread->getQueue()->emplaceCommand<Commands::MainToPrepare::SetOrientation>(id, v);
		return (*this);
	}

	RenderScene &RenderScene::setScale(const glm::vec3 &v, const PrepareKey &id)
	{
		_prepareThread->getQueue()->emplaceCommand<Commands::MainToPrepare::SetScale>(id, v);
		return (*this);
	}

	RenderScene &RenderScene::setCameraInfos(const PrepareKey &id
		, const glm::mat4 &projection)
	{
		_prepareThread->getQueue()->emplaceCommand<Commands::MainToPrepare::CameraInfos>(id, projection);
		return (*this);
	}

	RenderScene &RenderScene::setPosition(const glm::vec3 &v, const std::array<PrepareKey, MAX_CPT_NUMBER> &ids)
	{
		for (auto &e : ids)
			setPosition(v, e);
		return (*this);
	}

	RenderScene &RenderScene::setOrientation(const glm::quat &v, const std::array<PrepareKey, MAX_CPT_NUMBER> &ids)
	{
		for (auto &e : ids)
			setOrientation(v, e);
		return (*this);
	}

	RenderScene &RenderScene::setScale(const glm::vec3 &v, const std::array<PrepareKey, MAX_CPT_NUMBER> &ids)
	{
		for (auto &e : ids)
			setScale(v, e);
		return (*this);
	}

	RenderScene &RenderScene::updateGeometry(
		const PrepareKey &key
		, const AGE::Vector<AGE::SubMeshInstance> &meshs
		, const AGE::Vector<AGE::MaterialInstance> &materials
		, const gl::Key<AGE::AnimationInstance> &animation)
	{
		assert(!key.invalid() || key.type != PrepareKey::Type::Drawable);
		_prepareThread->getQueue()->emplaceCommand<Commands::MainToPrepare::SetGeometry>(key, meshs, materials, animation);
		return (*this);
	}

	DRAWABLE_ID RenderScene::addDrawable(USER_OBJECT_ID uid)
	{
		DRAWABLE_ID res = DRAWABLE_ID(-1);
		Drawable *co = nullptr;
		if (!_freeDrawables.empty())
		{
			res = _freeDrawables.front();
			_freeDrawables.pop();
			co = &(_drawables[res]);
		}
		else
		{
			res = _drawables.size();
			_drawables.emplace_back(Drawable());
			co = &(_drawables.back());
		}
		co->id = res;
		co->active = true;
		return res;
	}

	void RenderScene::removeDrawableObject(DRAWABLE_ID id)
	{
		_freeDrawables.push(PrepareKey::OctreeObjectId(id));
#ifdef ACTIVATE_OCTREE_CULLING
		// remove drawable from octree
		if (_drawables[id].toAddInOctree == false)
			_octree = _octree->removeElement(&_drawables[id]);
#endif
		_drawables[id].reset();
		assert(id != (std::size_t)(-1));
	}


	//bool RenderScene::_updateBegin()
	//{
	//	auto returnValue = true;

	//	return returnValue;
	//}

	//bool RenderScene::_updateEnd()
	//{
	//	auto returnValue = true;
	//	//_next->_next->getTaskQueue()->emplace<AGE::MTC::FrameTime>(_threadId, std::chrono::duration_cast<std::chrono::milliseconds>(_elapsed).count());
	//	return returnValue;
	//}

		void RenderScene::_setCameraInfos(AGE::Commands::MainToPrepare::CameraInfos &msg)
		{
			Camera *co = nullptr;
			co = &_cameras[msg.key.id];
			co->hasMoved = true;
			co->projection = msg.projection;
		}

		void RenderScene::_createCamera(AGE::Commands::MainToPrepare::CreateCamera &msg)
		{
			Camera *co = nullptr;
			if (msg.key.id >= _cameras.size())
			{
				_cameras.push_back(Camera());
				co = &_cameras.back();
			}
			else
				co = &_cameras[msg.key.id];
			co->key.id = msg.key.id;
			co->active = true;
		}

		void RenderScene::_createPointLight(AGE::Commands::MainToPrepare::CreatePointLight &msg)
		{
			PointLight *co = nullptr;
			if (msg.key.id >= _pointLights.size())
			{
				_pointLights.push_back(PointLight());
				co = &_pointLights.back();
			}
			else
				co = &_pointLights.back();
			co->key.id = msg.key.id;
			co->active = true;
		}

		void RenderScene::_createDrawable(AGE::Commands::MainToPrepare::CreateDrawable &msg)
		{
			Mesh *uo = nullptr;
			if (msg.key.id >= _meshs.size())
			{
				_meshs.push_back(Mesh());
				uo = &_meshs.back();
			}
			else
				uo = &_meshs[msg.key.id];
		}

		void RenderScene::_setPointLight(AGE::Commands::MainToPrepare::SetPointLight &msg)
		{
			PointLight *l = nullptr;
			l = &_pointLights[msg.key.id];
			l->color = msg.color;
			l->range = msg.range;
		}

		void RenderScene::_deleteCamera(AGE::Commands::MainToPrepare::DeleteCamera &msg)
		{
			Camera *co = nullptr;
			co = &_cameras[msg.key.id];
			co->active = false;
		}

		void RenderScene::_deletePointLight(AGE::Commands::MainToPrepare::DeletePointLight &msg)
		{
			PointLight *co = nullptr;
			co = &_pointLights[msg.key.id];
			co->active = false;
		}
		
		void RenderScene::_deleteDrawable(AGE::Commands::MainToPrepare::DeleteDrawable &msg)
		{
			Mesh *uo = nullptr;
			uo = &this->_meshs[msg.key.id];
			for (auto &e : uo->drawableCollection)
			{
				removeDrawableObject(e);
			}
			uo->drawableCollection.clear();
			uo->active = false;
		}

		void RenderScene::_setGeometry(AGE::Commands::MainToPrepare::SetGeometry &msg)
		{
			Mesh *uo = nullptr;
			uo = &_meshs[msg.key.id];
			for (auto &e : uo->drawableCollection)
				removeDrawableObject(e);
			uo->drawableCollection.clear();
			for (std::size_t i = 0; i < msg.submeshInstances.size(); ++i)
			{
				auto id = addDrawable(msg.key.id);
				uo->drawableCollection.push_back(id);
				_drawables[id].mesh = msg.submeshInstances[i];
				_drawables[id].material = msg.materialInstances[i];
				if (!msg.materialInstances[i])
				{
					std::cout << "lolkillme";
					assert(false);
				}
				_drawables[id].position = uo->position;
				_drawables[id].orientation = uo->orientation;
				_drawables[id].scale = uo->scale;
				_drawables[id].meshAABB = msg.submeshInstances[i].boundingBox;
				_drawables[id].toAddInOctree = true;
				_drawables[id].animation = msg.animation;
			}
		}

		void RenderScene::_setPosition(AGE::Commands::MainToPrepare::SetPosition &msg)
		{
			Camera *co = nullptr;
			Mesh *uo = nullptr;
			PointLight *l = nullptr;

			switch (msg.key.type)
			{

			case(PrepareKey::Type::Camera) :
				co = &_cameras[msg.key.id];
				co->position = msg.position;
				co->hasMoved = true;
				break;
			case(PrepareKey::Type::Drawable) :
				uo = &_meshs[msg.key.id];
				uo->position = msg.position;
				for (auto &e : uo->drawableCollection)
				{
					_drawables[e].position = uo->position;
					_drawables[e].hasMoved = true;
				}
				break;
			case(PrepareKey::Type::PointLight) :
				l = &_pointLights[msg.key.id];
				l->position = msg.position;
				break;
			default:
				break;
			}
		}

		void RenderScene::_setScale(AGE::Commands::MainToPrepare::SetScale &msg)
		{
			Mesh *uo = nullptr;
			Camera *co = nullptr;
			switch (msg.key.type)
			{
			case(PrepareKey::Type::Camera) :
				co = &_cameras[msg.key.id];
				co->scale = msg.scale;
				co->hasMoved = true;
				break;
			case(PrepareKey::Type::Drawable) :
				uo = &_meshs[msg.key.id];
				uo->scale = msg.scale;
				for (auto &e : uo->drawableCollection)
				{
					_drawables[e].scale = uo->scale;
					_drawables[e].hasMoved = true;
				}
				break;
			default:
				break;
			}
		}

		void RenderScene::_setOrientation(AGE::Commands::MainToPrepare::SetOrientation &msg)
		{
			Mesh *uo = nullptr;
			Camera *co = nullptr;
			switch (msg.key.type)
			{
			case(PrepareKey::Type::Camera) :
				co = &_cameras[msg.key.id];
				co->orientation = msg.orientation;
				co->hasMoved = true;
				break;
			case(PrepareKey::Type::Drawable) :
				uo = &_meshs[msg.key.id];
				uo->orientation = msg.orientation;
				for (auto &e : uo->drawableCollection)
				{
					_drawables[e].orientation = uo->orientation;
					_drawables[e].hasMoved = true;
				}
				break;
			default:
				break;
			}
		}
		
		void RenderScene::_prepareDrawList(AGE::Commands::MainToPrepare::PrepareDrawLists &msg)
		{
			AGE::Vector<CullableObject*> toDraw;


			// we update animation instances
			//auto animationManager = getDependencyManager().lock()->getInstance<AGE::AnimationManager>();
			//animationManager->update(0.1f);


			// Update drawable positions in Octree
			for (auto &e : _drawables)
			{
				if (e.hasMoved && e.toAddInOctree == false)
				{
					e.hasMoved = false;
					e.previousAABB = e.currentAABB;
					e.transformation = glm::scale(glm::translate(glm::mat4(1), e.position) * glm::toMat4(e.orientation), e.scale);
					e.currentAABB.fromTransformedBox(e.meshAABB, e.transformation);
					_octree = _octree->moveElement(&e);
				}
				if (e.toAddInOctree)
				{
					e.transformation = glm::scale(glm::translate(glm::mat4(1), e.position) * glm::toMat4(e.orientation), e.scale);
					e.currentAABB.fromTransformedBox(e.meshAABB, e.transformation);
					e.previousAABB = e.currentAABB;
					e.toAddInOctree = false;
					_octree = _octree->addElement(&e);
				}
			}
			// Do culling for each camera
			_octreeDrawList.clear();
			for (auto &camera : _cameras)
			{
				if (!camera.active)
					continue;
				
				auto view = glm::inverse(glm::scale(glm::translate(glm::mat4(1), camera.position) * glm::toMat4(camera.orientation), camera.scale));

				// update frustum infos for culling
				camera.currentFrustum.setMatrix(camera.projection * view);

				_octreeDrawList.emplace_back();
				auto &drawList = _octreeDrawList.back();
				drawList.transformation = view;
				drawList.projection = camera.projection;

				// no culling for the lights for the moment (TODO)
				for (size_t index = 0; index < _pointLights.size(); ++index)
				{
					auto &p = _pointLights[index];
					if (p.active)
						drawList.lights.emplace_back(p.position, p.color, p.range);
				}

#ifdef ACTIVATE_OCTREE_CULLING

				// Do the culling
				_octree->getElementsCollide(&camera, toDraw);

				// iter on element to draw
				for (CullableObject *e : toDraw)
				{
					// mandatory if you want the object to be found again
					e->hasBeenFound = false;
					// all the elements are drawable for the moment (TODO)
					Drawable *currentDrawable = dynamic_cast<Drawable*>(e);
					//if (!currentDrawable->animation.empty())
					//{
					//	drawList.drawables.emplace_back(currentDrawable->mesh, currentDrawable->material, currentDrawable->transformation, animationManager->getBones(currentDrawable->animation));
					//	drawList.drawables.back().animation = currentDrawable->animation;
					//}
					//else
					//{
						drawList.drawables.emplace_back(currentDrawable->mesh, currentDrawable->material, currentDrawable->transformation);
					//}
						if (!currentDrawable->material)
						{
							std::cout << "lol" << std::endl;
							assert(false);
						}
				}
#else
				for (auto &e : _drawables)
				{
					if (e.active)
					{
						drawList.drawables.emplace_back(e.mesh, e.material, e.transformation);
					}
				}
#endif
			}
			//getDependencyManager().lock()->getInstance<AGE::AnimationManager>()->update(0.1f);
				for (auto &i : this->_octreeDrawList)
				{
					for (auto &e : i.drawables)
					{
						if (!e.material)
						{
							std::cout << "lol";
							assert(false);
						}
					}
				}
			GetRenderThread()->getQueue()->emplaceCommand<Commands::Render::CopyDrawLists>(this->_octreeDrawList);
		}

}