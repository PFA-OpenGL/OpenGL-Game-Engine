#pragma once

#include <memory>
#include <Systems/System.h>
#include <Core/EntityFilter.hpp>

class Entity;

class SpaceshipControllerSystem : public System
{
public:
	SpaceshipControllerSystem(std::weak_ptr<AScene> &&scene);
	virtual ~SpaceshipControllerSystem();

	virtual void updateBegin(double time) { }
	virtual void updateEnd(double time) { }
	virtual void mainUpdate(double time);
	virtual bool initialize();

private:
	EntityFilter						_spaceshipsFilter;

	float _timer;

	std::queue<Entity> _bullets;
};
