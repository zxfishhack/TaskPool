#ifndef _TASK_POOL_THREAD_H_
#define _TASK_POOL_THREAD_H_
#ifdef _WIN32
#include <Windows.h>
#include <process.h>
#else
#include <pthread.h>
typedef unsigned long long KAFFINITY;
#endif

extern KAFFINITY REAL_CPU;

namespace Task {
	class Thread {
	public:
		typedef void(*entry_function)(void*);
		Thread(entry_function entry, void* ctx, size_t stackSize = 1024*1024, KAFFINITY affininity = 0)
		:m_entry(entry), m_ctx(ctx) {
#ifdef _WIN32
			unsigned thrId;
			m_handle = HANDLE(_beginthreadex(NULL, (unsigned)stackSize, s_routine, this, CREATE_SUSPENDED, &thrId));
			if (m_handle) {
				if (affininity) {
					::SetThreadAffinityMask(m_handle, affininity);
				}
				::ResumeThread(m_handle);
			}
#else
			pthread_attr_t attr;
			pthread_attr_init(&attr);
			if (affininity) {
				cpu_set_t cpu_info;
				CPU_ZERO(&cpu_info);
				KAFFINITY _1(1);
				for (size_t i = 0; i<sizeof(affininity) * 8; i++) {
					if ((_1 << i) & affininity) {
						CPU_SET(i, &cpu_info);
					}
				}
				pthread_attr_setaffinity_np(&attr, sizeof(cpu_info), &cpu_info);
			}
			if (stackSize) {
				pthread_attr_setstacksize(&attr, stackSize);
			}
			pthread_create(&m_thread, &attr, s_routine, this);
#endif
		}
		void join() const {
#ifdef _WIN32
			WaitForSingleObject(m_handle, INFINITE);
#else
			void * retval;
			pthread_join(m_thread, &retval);
#endif
		}
	private:
#ifdef _WIN32
		HANDLE m_handle;
		static unsigned __stdcall s_routine(void* _self) {
#else
		pthread_t m_thread;
		static void * s_routine(void* _self) {
#endif
			Thread* self = reinterpret_cast<Thread*>(_self);
			self->m_entry(self->m_ctx);
			return 0;
		}
		entry_function m_entry;
		void* m_ctx;
	};
}

#endif
