#include <global.h>
#include <coroutine/coroutine.h>
#include <coroutine/task.h>
#include <coroutine/coroutineschedule.h>
#include <taskpool.h>

namespace Task {
#ifdef _WIN32
	Coroutine::Coroutine(ITask *task) 
		: m_task(task)
		, m_status(READY) {
		m_fiber = ::CreateFiberEx(0, 0, FIBER_FLAG_FLOAT_SWITCH, s_fiberProc, this);
	}
	Coroutine::~Coroutine() {
		::DeleteFiber(m_fiber);
	}
	void Coroutine::s_fiberProc(LPVOID p) {
		static_cast<Coroutine*>(p)->fiberProc();
	}
#endif

#ifdef __linux__
	Coroutine::Coroutine(ITask *task)
		: m_task(task)
		, m_status(READY) {
		m_stack = new char[STACK_SIZE];
		getcontext(&m_ctx);
		m_ctx.uc_stack.ss_sp = m_stack;
		m_ctx.uc_stack.ss_size = STACK_SIZE;
		m_ctx.uc_link = NULL;
		uintptr_t ptr = (uintptr_t)this;
		makecontext(&m_ctx, (void(*)(void))Coroutine::s_fiberProc, 2, (uint32_t)ptr, (uint32_t)(ptr >> 32));
	}
	Coroutine::~Coroutine() {
		delete[]m_stack;
	}
	void Coroutine::s_fiberProc(uint32_t low32, uint32_t hi32) {
		uintptr_t ptr = (uintptr_t)low32 | ((uintptr_t)hi32 << 32);
		reinterpret_cast<Coroutine*>(ptr)->fiberProc();
	}
#endif

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
			if (m_task) {
				m_task->Run();
				m_status = READY;
				yield();
			}
		}
	}
}
