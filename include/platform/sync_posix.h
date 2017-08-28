#include <global.h>
#include <exception>
#include <pthread.h>
#include <semaphore.h>

namespace Task {

	class Mutex : boost::noncopyable {
	public:
		Mutex() {
			pthread_mutexattr_t attr;
			pthread_mutexattr_init(&attr);
			pthread_mutex_init(&m_mutex, &attr);
		}
		~Mutex() {
			pthread_mutex_destroy(&m_mutex);
		}
		void lock() {
			pthread_mutex_lock(&m_mutex);
		}
		void unlock() {
			pthread_mutex_unlock(&m_mutex);
		}
	private:
		pthread_mutex_t m_mutex;
	};

	class Semaphore : boost::noncopyable {
	public:
		Semaphore(int maxVal = 4096) {
			sem_init(&m_semaphore, 0, 0);
		}
		~Semaphore() {
			sem_destroy(&m_semaphore);
		}
		void up(int count = 1) {
			for(auto i=0; i<count; i++) {
				sem_post(&m_semaphore);
			}
		}
		void down() {
			sem_wait(&m_semaphore);
		}
	private:
		sem_t m_semaphore;
	};
}

