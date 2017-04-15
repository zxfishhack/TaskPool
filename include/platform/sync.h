#ifndef _TASK_POOL_SYNC_H_
#define _TASK_POOL_SYNC_H_

#include <global.h>
#include <exception>

namespace Task {

	class Mutex {
	public:
		Mutex() {
			m_mutex = ::CreateMutexA(NULL, FALSE, NULL);
			if (m_mutex == NULL) {
				throw(std::bad_alloc());
			}
		}
		~Mutex() {
			::CloseHandle(m_mutex);
			m_mutex = NULL;
		}
		void lock() const {
			DWORD ret;
			do {
				ret = ::WaitForSingleObjectEx(m_mutex, INFINITE, TRUE);
			} while (ret != WAIT_OBJECT_0);
		}
		void unlock() const {
			::ReleaseMutex(m_mutex);
		}
	private:
		HANDLE m_mutex;
	};

	class Semaphore {
	public:
		Semaphore(int maxVal = 4096) {
			m_semaphore = ::CreateSemaphoreA(NULL, 0, maxVal, NULL);
			if (m_semaphore == NULL) {
				throw(std::bad_alloc());
			}
		}
		~Semaphore() {
			::CloseHandle(m_semaphore);
		}
		void up(int count = 1) const {
			::ReleaseSemaphore(m_semaphore, count, NULL);
		}
		void down() const {
			DWORD ret;
			do {
				ret = ::WaitForSingleObjectEx(m_semaphore, INFINITE, TRUE);
			} while (ret != WAIT_OBJECT_0);
		}
	private:
		HANDLE m_semaphore;
	};

	class ScopedLock {
	public:
		ScopedLock(const Mutex& mtx) : m_mutex(mtx) {
			m_mutex.lock();
		}
		~ScopedLock() {
			m_mutex.unlock();
		}
	private:
		const Mutex& m_mutex;
	};
}

#endif
