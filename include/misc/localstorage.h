#ifndef _TASK_POOL_LOCAL_STORAGE_H_
#define _TASK_POOL_LOCAL_STORAGE_H_

#include <global.h>
#include <new>
#ifdef __linux__
#include <pthread.h>
#endif

namespace Task {
	namespace detail {
#ifdef _WIN32
		template<typename Ty>
		class ThreadLocal : boost::noncopyable {
		public:
			ThreadLocal() {
				m_tlsId = ::TlsAlloc();
			}
			~ThreadLocal() {
				::TlsFree(m_tlsId);
			}
			void set(Ty p) {
				::TlsSetValue(m_tlsId, LPVOID(p));
			}
			Ty get() {
				return reinterpret_cast<Ty>(::TlsGetValue(m_tlsId));
			}
		private:
			DWORD m_tlsId;
		};
#endif

#ifdef __linux__
		template<typename Ty>
		class ThreadLocal {
		public:
			ThreadLocal() {
				if (pthread_key_create(&m_key, NULL) != 0) {
					throw std::bad_alloc();
				}
				set(Ty(0));
			}
			~ThreadLocal() {
				pthread_key_delete(m_key);
			}
			void set(Ty p) const {
				pthread_setspecific(m_key, (const void*)p);
			}
			Ty get() const {
				return Ty(pthread_getspecific(m_key));
			}
		private:
			pthread_key_t m_key;
		};
#endif
	}
}

#endif
