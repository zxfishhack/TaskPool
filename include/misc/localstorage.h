#ifndef _TASK_POOL_LOCAL_STORAGE_H_
#define _TASK_POOL_LOCAL_STORAGE_H_

#include <global.h>

namespace Task {
	namespace detail {
		
		template<typename Ty>
		class ThreadLocal {
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
		
		template<typename Ty>
		class CoroutineLocal {
		public:
			CoroutineLocal() {
				m_clsId = ::FlsAlloc(NULL);
			}
			~CoroutineLocal() {
				::FlsFree(m_clsId);
			}
			void set(Ty p) {
				::FlsSetValue(m_clsId, p);
			}
			Ty get() {
				return static_cast<Ty>(::FlsGetValue(m_clsId));
			}
		private:
			DWORD m_clsId;
		};
	}
}

#endif
