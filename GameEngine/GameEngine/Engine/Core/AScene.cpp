
#include "AScene.hh"
#include <Core/Engine.hh>
#include <Entities/EntityManager.h>

AScene::AScene(Engine &engine) :
	_camera(NULL),
	_engine(engine)
{
}

AScene::~AScene()
{
	if (_camera)
		delete _camera;
	_systems.clear();
}


// to delete !!!
void 							AScene::recomputePositions(Handle &father,
															bool hasMoved)
{
	if (!father.get())
		return;
	Entity::t_sonsList::iterator 	it = father->getSonsBegin();

	while (it != father->getSonsEnd())
	{

		if ((it->get())->getFlags() & Entity::HAS_MOVED)
			hasMoved = true;
		if (hasMoved)
			it->get()->computeGlobalTransform(father->getGlobalTransform());
		auto &a = *it;
		recomputePositions(it->get()->getHandle(), hasMoved);
		++it;
	}
}

void 							AScene::setCamera(ACamera *camera)
{
	_camera = camera;
}

ACamera 						*AScene::getCamera() const
{
	return (_camera);
}

void 							AScene::update(double time)
{
	for (auto &e : _systems)
	{
		e.second->update(time);
	}
	//_root->computeGlobalTransform(glm::mat4(1));
	//recomputePositions(_engine.getInstance<EntityManager>().getRoot(), false);
}