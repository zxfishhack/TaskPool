#include <global.h>
#include <coroutine/coroutineschedule.h>
#include <coroutine/coroutine.h>

namespace Task {
#ifdef _WIN32
	CoroutineSchedule::CoroutineSchedule() {
		m_fiber = ::ConvertThreadToFiber(NULL);
		m_running = NULL;
	}

	CoroutineSchedule::~CoroutineSchedule() {
		::ConvertFiberToThread();
	}
#endif

#ifdef __linux__
	CoroutineSchedule::CoroutineSchedule() {
		m_stack = new char[STACK_SIZE];
		m_running = NULL;
	}

	CoroutineSchedule::~CoroutineSchedule() {
		delete[]m_stack;
	}
#endif

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
#ifdef _WIN32
		::SwitchToFiber(m_fiber);
#endif
#ifdef __linux__
		swapcontext(&co->m_ctx, &m_main);
#endif
	}

	void CoroutineSchedule::resume(Coroutine *co) {
		co->m_status = Coroutine::RUNNING;
		m_running = co;
#ifdef _WIN32
		::SwitchToFiber(co->m_fiber);
#endif
#ifdef __linux__
		swapcontext(&m_main, &co->m_ctx);
#endif
	}
	Coroutine * CoroutineSchedule::running() const {
		return m_running;
	}
}
