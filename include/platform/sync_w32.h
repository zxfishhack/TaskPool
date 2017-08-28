
#include <global.h>
#include <exception>

namespace Task {

	class Mutex : boost::noncopyable {
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
		void lock() {
			DWORD ret;
			do {
				ret = ::WaitForSingleObjectEx(m_mutex, INFINITE, TRUE);
			} while (ret != WAIT_OBJECT_0);
		}
		void unlock() {
			::ReleaseMutex(m_mutex);
		}
	private:
		HANDLE m_mutex;
	};

	class Semaphore : boost::noncopyable {
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
		void up(int count = 1) {
			::ReleaseSemaphore(m_semaphore, count, NULL);
		}
		void down() {
			DWORD ret;
			do {
				ret = ::WaitForSingleObjectEx(m_semaphore, INFINITE, TRUE);
			} while (ret != WAIT_OBJECT_0);
		}
	private:
		HANDLE m_semaphore;
	};
}
