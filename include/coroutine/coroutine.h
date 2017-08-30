#ifndef _TASK_POOL_COROUTINE_H_
#define _TASK_POOL_COROUTINE_H_

#ifdef __linux__
#include <ucontext.h>
#include <stdint.h>
#endif

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
		void setWaking(bool waking);
		bool waking() const;
		Status status() const;
		int managedThreadId() const;
		ITask* task();
	private:
		friend class CoroutineSchedule;
		ITask* m_task;
		Status m_status;
		bool m_waking;
		void fiberProc();
#ifdef _WIN32
		HANDLE m_fiber;
		static void __stdcall s_fiberProc(LPVOID p);
#endif

#ifdef __linux__
		ucontext_t m_ctx;
		char* m_stack;
		static void s_fiberProc(uint32_t low32, uint32_t hi32);
#endif
	};
};

#endif
