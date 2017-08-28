#ifndef _TASK_POOL_SYNC_H_
#define _TASK_POOL_SYNC_H_

#ifdef _WIN32
#include "sync_w32.h"
#endif

#ifdef __linux__
#include "sync_posix.h"
#endif

namespace Task {
	class ScopedLock : boost::noncopyable {
	public:
		ScopedLock(Mutex& mtx) : m_mutex(mtx) {
			m_mutex.lock();
		}
		~ScopedLock() {
			m_mutex.unlock();
		}
	private:
		Mutex& m_mutex;
	};
}

#endif
