#ifndef _TASK_POOL_COROUTINE_H_
#define _TASK_POOL_COROUTINE_H_

namespace Task {
	
	class ITask;
	class CoroutineSchedule;

	class Coroutine : boost::noncopyable {
	public:
		enum Status {
			DEAD,
			READY,
			RUNNING,
			WAITING,
			SUSPEND
		};

		Coroutine(ITask* task);
		~Coroutine();
		void reset(ITask* task);
		void yield();
		void resume();
		void setWaiting();
		Status status() const;
		int managedThreadId() const;
		ITask* task();
	private:
		friend class CoroutineSchedule;
		ITask* m_task;
		Status m_status;
		HANDLE m_fiber;
		void fiberProc();
		static void __stdcall s_fiberProc(LPVOID p);
	};
};

#endif
