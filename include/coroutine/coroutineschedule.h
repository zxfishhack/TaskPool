#ifndef _TASK_POOL_COROUTINE_SCHEDULE_H_
#define _TASK_POOL_COROUTINE_SCHEDULE_H_

#include <global.h>
#include "coroutine.h"

namespace Task {

	class Task;
	class Coroutine;

	class CoroutineSchedule {
	public:
		CoroutineSchedule();
		~CoroutineSchedule();
		void yield(Coroutine *co);
		void resume(Coroutine *co);

		Coroutine* running() const;
	private:
#ifdef _WIN32
		HANDLE m_fiber;
#endif
#ifdef __linux__
		ucontext_t m_main;
		char * m_stack;
#endif
		Coroutine *m_running;
	};

}

#endif
