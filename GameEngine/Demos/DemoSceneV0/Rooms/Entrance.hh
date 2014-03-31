#pragma once

#include "Room.hpp"
#include <Utils/PubSub.hpp>

struct Entrance : public Room
{
	Entrance(std::weak_ptr<AScene> &&scene);
	virtual ~Entrance();
	virtual bool init();
	Entity room;
	Entity pong;
	Entity hotZone;
	std::vector<Entity> lights;
	PubSub framebufferIdReceiver;
	Entity fboTest;
	Entity welcomeText;
protected:
	virtual bool _enable();
	virtual bool _disable();
};