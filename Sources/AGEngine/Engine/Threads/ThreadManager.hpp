#pragma once

#include "Thread.hpp"
#include <Utils/Singleton.hh>
#include <Utils/Containers/Vector.hpp>
#include <memory>
#include <atomic>
#include <array>
#include <Utils/SpinLock.hpp>

namespace TMQ
{
	class HybridQueue;
}

namespace AGE
{
	class MainThread;
	class RenderThread;
	class Engine;

	class ThreadManager
	{
	public:
		Thread *getCurrentThread() const;
		MainThread *getMainThread() const;
		RenderThread *getRenderThread() const;
		Engine *createEngine();
		Engine *getEngine();
		bool initAndLaunch();
		void exit();
		void updateThreadStatistics(Thread::ThreadType type, std::size_t workTime, std::size_t waitTime);
		struct ThreadStatistics
		{
			std::string name;
			std::atomic<float> averageWorkTime;
			std::atomic<float> averageWaitTime;
			float averageWorkTimeCopy;
			float averageWaitTimeCopy;
			std::atomic_size_t frameCounter;
			std::atomic_size_t secondCounter;
			std::array < std::size_t, 128 > work;
			std::array < std::size_t, 128 > wait;
			std::array < float, 128 > workCopy;
			std::array < float, 128 > waitCopy;
			std::array < float, 128 > totalCopy;
			ThreadStatistics();
		};
		inline std::array<ThreadStatistics, Thread::END> &getStatistics()
		{
			return _threadsStatistics;
		}

		TMQ::HybridQueue *getAvailableTaskQueue(bool futur = false, Thread::ThreadType type = Thread::END);

		void forEachThreads(std::function<void(AGE::Thread *)> &&fn);
		void setAsWorker(bool mainThread, bool prepareThread, bool renderThread);
	private:
		std::array<ThreadStatistics, Thread::END> _threadsStatistics;
		Engine *_engine;
		SpinLock _mutex;
		std::size_t _iterator = Thread::Main;

		
		ThreadManager();
		virtual ~ThreadManager();
		ThreadManager(const ThreadManager &o) = delete;
		ThreadManager(ThreadManager &&o) = delete;
		ThreadManager &operator=(const ThreadManager &o) = delete;
		ThreadManager &operator=(ThreadManager &&o) = delete;
		void registerThreadId(std::size_t systemHash, Thread::ThreadType type);

		AGE::Vector<AGE::Thread*> _threads;
		AGE::Vector<std::size_t>  _threadIdReference;

		friend class Singleton < ThreadManager > ;
		friend class Thread;
	};

	ThreadManager *GetThreadManager();
	Thread *CurrentThread();
	MainThread *GetMainThread();
	RenderThread *GetRenderThread();
	bool InitAGE();
	void ExitAGE();
}