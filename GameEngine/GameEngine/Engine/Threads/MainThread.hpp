#pragma once

#include "Thread.hpp"
#include "QueuePusher.hpp"
#include "QueueOwner.hpp"
#include <Utils/Containers/Vector.hpp>
#include <memory>


namespace AGE
{
	class Engine;

	class MainThread : public Thread, public QueuePusher, public QueueOwner
	{
	public:
		virtual bool init();
		virtual bool release();
		virtual bool launch();
		virtual bool stop();
		bool run();
		bool update();
		std::weak_ptr<AGE::Engine> createEngine();
		std::weak_ptr<AGE::Engine> getEngine();
	private:
		MainThread();
		virtual ~MainThread();
		MainThread(const MainThread &) = delete;
		MainThread(MainThread &&) = delete;
		MainThread &operator=(const MainThread &) = delete;
		MainThread &operator=(MainThread &&) = delete;
		std::atomic_bool _run;

		friend class ThreadManager;

		std::shared_ptr<AGE::Engine> _engine;
	};
}