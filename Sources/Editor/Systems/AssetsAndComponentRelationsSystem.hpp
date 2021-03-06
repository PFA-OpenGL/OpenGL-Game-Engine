#pragma once

#include <System/System.h>

namespace AGE
{
	namespace WE
	{
		class AssetsAndComponentRelationsSystem : public System<AssetsAndComponentRelationsSystem>
		{
		public:
			AssetsAndComponentRelationsSystem(AScene *scene);
			virtual ~AssetsAndComponentRelationsSystem();
		private:
			EntityFilter _rigidBodies;
			EntityFilter _meshRenderer;
			EntityFilter _all;

			virtual void updateBegin(float time);
			virtual void updateEnd(float time);
			virtual void mainUpdate(float time);
			virtual bool initialize();
		};
	}
}