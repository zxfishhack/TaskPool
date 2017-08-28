#ifndef _TASK_POOL_TASK_H_
#define _TASK_POOL_TASK_H_

#include <global.h>
#include <boost/function.hpp>

namespace Task {

	class Coroutine;

	class ITask : boost::noncopyable {
	public:
		void Run();

		virtual ~ITask() = 0 {};
		virtual void run() = 0;
		bool cancelTask();
		void associate(Coroutine* co);
		Coroutine* associate() const;
		void associateThread(size_t thrId);
		size_t associateThread() const;
		void setCancelHandle(size_t cancelHandle);
		size_t cancelHandle() const;

	protected:
		ITask()
			: m_co(NULL)
			, m_thrId(0)
			, m_cancelHandle(0)
		{}
		Coroutine* m_co;

		size_t m_thrId;
		size_t m_cancelHandle;
	};

	class Task : public ITask {
	public:
		static ITask* create(boost::function<void()> func);
		static void destory(ITask* task);
	private:
		boost::function<void()> m_func;

		Task(boost::function<void()> func) 
			: m_func(func) {}

		void run() {
			m_func();
		}
	};
	class Pool;

	class PromiseNotify : boost::noncopyable {
	public:
		PromiseNotify(Coroutine* taskToNotify, Pool* pool)
			: m_taskToNotify(taskToNotify)
			, m_pool(pool){}

		bool isCancelled();
		void notify(tribool res);
	private:
		Coroutine* m_taskToNotify;
		Pool* m_pool;
	};
}

#endif
