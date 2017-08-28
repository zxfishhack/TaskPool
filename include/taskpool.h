#ifndef _TASK_POOL_H_
#define _TASK_POOL_H_

#include <misc/localstorage.h>
#include <promise/promise.h>
#include <exception>
#include <coroutine/coroutine.h>
#include <platform/sync.h>
#include <platform/thread.h>
#include <deque>
#include <coroutine/task.h>
#include <boost/bind.hpp>
#include <unordered_map>
#include <boost/atomic.hpp>
#include <vector>
#include <promise/deferredcontext.h>
#include <coroutine/coroutineschedule.h>

namespace Task {

	class ITask;
	template<typename Ret>
	class PromiseTask;
	class CoroutineSchedule;
	class Pool;
	
	namespace internal {
		extern detail::ThreadLocal<Pool*> curPool;
		extern detail::ThreadLocal<CoroutineSchedule*> curSchedule;
		extern detail::ThreadLocal<size_t> curThreadId;
	}
	
	class Pool : boost::noncopyable {
	public:
		Pool(int thrNum, KAFFINITY affinity = 0);
		~Pool();
		void join();
		bool addTask(ITask* task, size_t* cancelHandle = 0);
		bool addImmediatelyTask(ITask* task, size_t* cancelHandle = 0);
		bool cancelTask(size_t cancelHandle);
		size_t poolId() const;
	private:
		bool wakeupCoroutine(Coroutine * co);

		typedef std::deque<Coroutine*> TaskQueueType;
		typedef std::unordered_map<size_t, ITask*> CancelableTaskMap;
		std::vector<Thread*> m_threads;
		int m_threadNum;
		size_t m_poolId;
		static boost::atomic<size_t> s_poolId;
		boost::atomic<size_t> m_thrId;
		boost::atomic<size_t> m_curIdx;
		volatile bool m_exit;
		volatile bool m_exited;
		Mutex* m_lock;
		Semaphore* m_semaphore;
		TaskQueueType* m_queue;
		TaskQueueType* m_privateQueue;
		Mutex m_freeLock;
		TaskQueueType m_freeQueue;
		boost::atomic<size_t> m_freeCount;
		Mutex m_cancelLock;
		CancelableTaskMap m_cancelable;
		boost::atomic<size_t> m_cancelHandleValue;
		static const size_t MAX_FREE_QUEUE_SIZE = 1024;
		void routine();
		static void s_routine(void* ptr);
		Coroutine * allocCoroutine(size_t idx, ITask* task);
		void freeCoroutine(Coroutine* co);
		static void checkSemaphore(TaskQueueType& list, Semaphore& sem);
		bool removeTask(ITask* task);
		friend class PromiseNotify;
	};

	class CancelAsyncException : public std::exception {};

	#include "detail/taskpool_impl.h"

	Promise<void> createTimeout(unsigned int timeoutMs);

	template<typename function>
	Promise<typename function::result_type> async(function func) {
		using Ret = typename function::result_type;
		Pool* pool = internal::curPool.get();
		if (pool == NULL) {
			return Promise<Ret>(false);
		}
		PromiseTask<Ret> * task = new PromiseTask<Ret>(func);
		// get promise before addTask, prevent task delete after executed.
		Promise<Ret> pro = task->promise();
		if (!pool->addTask(task)) {
			delete task;
			return Promise<Ret>(false);
		} else {
			return pro;
		}
	}

	template<typename Ty>
	Promise<Ty> await(Promise<Ty> pro) {
		Pool* pool = internal::curPool.get();
		ITask* task = internal::curSchedule.get()->running()->task();
		Coroutine* co = task->associate();
		assert(co);
		if (indeterminate(pro.reset())) {
			boost::shared_ptr<PromiseNotify> notify(new PromiseNotify(co, pool));
			pro.always(boost::bind(&PromiseNotify::notify, notify.get(), boost::placeholders::_1));
			co->setWaiting();
			co->yield();
			pro.removeAlways();
			if (notify->isCancelled()) {
				pro.cancel();
				throw(CancelAsyncException());
			}
		}
		return pro;
 	}

	Promise<void> waitAll(std::vector<IPromise> promises, unsigned int timeoutMs, CancelToken token);
	Promise<void> waitAll(std::vector<IPromise> promises, unsigned int timeoutMs);
	Promise<void> waitAll(std::vector<IPromise> promises);

	Promise<void> waitAny(std::vector<IPromise> promises, unsigned int timeoutMs, CancelToken token);
	Promise<void> waitAny(std::vector<IPromise> promises, unsigned int timeoutMs);
	Promise<void> waitAny(std::vector<IPromise> promises);
}

#endif
