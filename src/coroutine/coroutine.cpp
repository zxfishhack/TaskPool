#include <global.h>
#include <coroutine/coroutine.h>
#include <coroutine/task.h>
#include <coroutine/coroutineschedule.h>
#include <taskpool.h>

namespace Task {

	Coroutine::Coroutine(ITask *task) 
		: m_task(task)
		, m_status(READY) {
		m_fiber = ::CreateFiberEx(0, 0, FIBER_FLAG_FLOAT_SWITCH, s_fiberProc, this);
	}
	Coroutine::~Coroutine() {
		::DeleteFiber(m_fiber);
	}

	void Coroutine::reset(ITask *task) {
		m_status = READY;
		m_task = task;
	}

	void Coroutine::yield() {
		internal::curSchedule.get()->yield(this);
	}

	void Coroutine::resume() {
		internal::curSchedule.get()->resume(this);
	}

	void Coroutine::setWaiting() {
		m_status = WAITING;
	}

	Coroutine::Status Coroutine::status() const {
		return m_status;
	}

	int Coroutine::managedThreadId() const {
		return (int)m_task->associateThread();
	}

	ITask * Coroutine::task() {
		return m_task;
	}

	void Coroutine::fiberProc() {
		while(m_status != DEAD) {
			internal::curTask.set(m_task);
			if (m_task) {
				m_task->Run();
				m_status = READY;
				internal::curTask.set(NULL);
				yield();
			}
		}
	}

	void Coroutine::s_fiberProc(LPVOID p) {
		static_cast<Coroutine*>(p)->fiberProc();
	}
}
