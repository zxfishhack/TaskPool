#include <global.h>
#include <coroutine/coroutineschedule.h>
#include <coroutine/coroutine.h>

namespace Task {

	CoroutineSchedule::CoroutineSchedule() {
		m_fiber = ::ConvertThreadToFiber(NULL);
		m_running = NULL;
	}

	CoroutineSchedule::~CoroutineSchedule() {
		::ConvertFiberToThread();
	}

	void CoroutineSchedule::yield(Coroutine *co) {
		m_running = NULL;
		switch (co->m_status) {
		case Coroutine::DEAD:
		case Coroutine::WAITING:
		case Coroutine::READY:
			break;
		default:
			co->m_status = Coroutine::SUSPEND;
		}
		::SwitchToFiber(m_fiber);
	}

	void CoroutineSchedule::resume(Coroutine *co) {
		co->m_status = Coroutine::RUNNING;
		m_running = co;
		::SwitchToFiber(co->m_fiber);
	}
	Coroutine * CoroutineSchedule::running() const {
		return m_running;
	}
}
