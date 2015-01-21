#ifndef __TASKS_MAIN_TO_PREPARE__
#define __TASKS_MAIN_TO_PREPARE__

#include <TMQ/message.hpp>

namespace AGE
{
	class AScene;
	namespace Tasks
	{
		namespace MainToPrepare
		{
			struct CreateScene : public TMQ::FutureData<bool>
			{
				CreateScene(AScene *_scene);
				AScene *scene;
			};
		};
	}
}
#endif