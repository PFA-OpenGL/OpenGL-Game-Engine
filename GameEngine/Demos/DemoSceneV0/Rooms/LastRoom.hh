#pragma once

#include "Room.hpp"

struct LastRoom : public Room
{
	LastRoom(std::weak_ptr<AScene> scene);
	virtual ~LastRoom();
	virtual bool init();
	Entity hotZoneLastEngine;
protected:
	virtual bool _enable();
	virtual bool _disable();
};