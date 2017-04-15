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
		HANDLE m_fiber;
		Coroutine *m_running;
	};

}

#endif
