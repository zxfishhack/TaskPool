#include <global.h>
#include <taskpool.h>
#include <coroutine/coroutine.h>
#include <coroutine/coroutineschedule.h>
#include <boost/bind/placeholders.hpp>
#include <boost/asio.hpp>
#include <boost/date_time.hpp>
#include "platform/singleton.h"

namespace Task {

	namespace internal {
		detail::ThreadLocal<Pool*> curPool;
		detail::ThreadLocal<CoroutineSchedule*> curSchedule;
		detail::ThreadLocal<size_t> curThreadId;

		int msb(KAFFINITY n) {
			int msb = -1;
			while(n!=0) {
				msb++;
				n >>= 1;
			}
			return msb;
		}

		int lsb(KAFFINITY n) {
			if (n == 0) {
				return -1;
			}
			int lsb = 0;
			while((n&1) == 0) {
				n >>= 1;
				lsb++;
			}
			return lsb;
		}
	}

	boost::atomic<size_t> Pool::s_poolId(0);

	Pool::Pool(int thrNum, KAFFINITY affinity)
		: m_threadNum(thrNum)
		, m_thrId(0)
		, m_curIdx(0)
		, m_exit(false)
		, m_exited(false)
		, m_freeCount(0)
		, m_cancelHandleValue(0) {
		m_threads.resize(m_threadNum);
		m_lock = new Mutex[m_threadNum];
		m_queue = new TaskQueueType[m_threadNum];
		m_privateQueue = new TaskQueueType[m_threadNum];
		m_semaphore = new Semaphore[m_threadNum];
		int maxIdx = internal::msb(affinity);
		int minIdx = internal::lsb(affinity);
		KAFFINITY prefAffinity = 0;
		int curIdx = minIdx;
		m_poolId = s_poolId.fetch_add(1) + 1;
		for (int i = 0; i < m_threadNum; i++) {
			if (affinity != 0) {
				prefAffinity = KAFFINITY(1) << curIdx;
				while(prefAffinity & affinity) {
					curIdx++;
					if (curIdx > maxIdx) {
						curIdx = minIdx;
					}
					prefAffinity = KAFFINITY(1) << curIdx;
				}
			}
			m_threads[i] = new Thread(Pool::s_routine, this, STACK_SIZE + 1024, prefAffinity);
			curIdx++;
		}
	}

	Pool::~Pool() {
		join();
		for (int i = 0; i < m_threadNum; i++) {
			delete m_threads[i];
			for(TaskQueueType::iterator it = m_queue[i].begin(); it != m_queue[i].end(); ++it) {
				delete *it;
			}
			for (TaskQueueType::iterator it = m_privateQueue[i].begin(); it != m_privateQueue[i].end(); ++it) {
				delete *it;
			}
		}
		for(TaskQueueType::iterator it = m_freeQueue.begin(); it != m_freeQueue.end(); ++it) {
			delete *it;
		}
		delete[] m_lock;
		delete[] m_queue;
		delete[] m_privateQueue;
		delete[] m_semaphore;
	}
	void Pool::join() {
		m_exit = true;
		for (int i = 0; i < m_threadNum; i++) {
			m_semaphore[i].up();
		}
		for(int i = 0; i < m_threadNum; i++) {
			m_threads[i]->join();
		}
		m_exited = true;
	}
	bool Pool::addTask(ITask *task, size_t *cancelHandle) {
		if (m_exited) {
			return false;
		}
		size_t curIdx = m_curIdx.fetch_add(1);
		curIdx %= m_threadNum;
		try {
			Coroutine* co = allocCoroutine(curIdx, task);
			if (cancelHandle) {
				*cancelHandle = m_cancelHandleValue.fetch_add(1);
				if (*cancelHandle == 0) {
					*cancelHandle = m_cancelHandleValue.fetch_add(1);
				}
				task->setCancelHandle(*cancelHandle);
			}
			if (cancelHandle) {
				ScopedLock _(m_cancelLock);
				m_cancelable[*cancelHandle] = task;
			}
			{
				ScopedLock _(m_lock[curIdx]);
				m_queue[curIdx].push_back(co);
				checkSemaphore(m_queue[curIdx], m_semaphore[curIdx]);
			}
			return true;
		}
		catch (...) {
			return false;
		}
	}
	bool Pool::addImmediatelyTask(ITask *task, size_t *cancelHandle) {
		if (m_exited) {
			return false;
		}
		size_t curIdx = m_curIdx.fetch_add(1);
		curIdx %= m_threadNum;
		try {
			Coroutine* co = allocCoroutine(curIdx, task);
			if (cancelHandle) {
				*cancelHandle = m_cancelHandleValue.fetch_add(1);
				if (*cancelHandle == 0) {
					*cancelHandle = m_cancelHandleValue.fetch_add(1);
				}
				task->setCancelHandle(*cancelHandle);
			}
			if (cancelHandle) {
				ScopedLock _(m_cancelLock);
				m_cancelable[*cancelHandle] = task;
			}
			{
				ScopedLock _(m_lock[curIdx]);
				m_queue[curIdx].push_front(co);
				checkSemaphore(m_queue[curIdx], m_semaphore[curIdx]);
			}
			return true;
		}
		catch (...) {
			return false;
		}
	}
	bool Pool::cancelTask(size_t cancelHandle) {
		if (m_exited) {
			return false;
		}
		if (cancelHandle == 0) {
			return false;
		}
		ITask* task;
		{
			ScopedLock _(m_cancelLock);
			CancelableTaskMap::iterator it = m_cancelable.find(cancelHandle);
			if (it == m_cancelable.end()) {
				return false;
			}
			task = it->second;
			m_cancelable.erase(it);
		}
		return removeTask(task);
	}

	size_t Pool::poolId() const {
		return m_poolId;
	}

	bool Pool::wakeupCoroutine(Coroutine *co) {
		if (m_exited) {
			return false;
		}
		if (!co) {
			return false;
		}
		if (co->task()->associateThread() == 0) {
			// don't wakeup coroutine before first run
			return false;
		}
		size_t idx = co->task()->associateThread() - 1;
		{
			// always wakeup in same thread
			ScopedLock _(m_lock[idx]);
			m_privateQueue[idx].push_front(co);
			checkSemaphore(m_privateQueue[idx], m_semaphore[idx]);
			return true;
		}
	}
	void Pool::routine() {
		CoroutineSchedule cs;
		internal::curPool.set(this);
		internal::curSchedule.set(&cs);
		size_t idx = m_thrId.fetch_add(1);
		internal::curThreadId.set(idx + 1);
		while (!m_exit) {
			Coroutine* co = NULL;
			{
				ScopedLock _(m_lock[idx]);
				if (!m_queue[idx].empty()) {
					co = m_queue[idx].front();
					m_queue[idx].pop_front();
				}
				if (!co) {
					if (!m_privateQueue[idx].empty()) {
						co = m_privateQueue[idx].front();
						m_privateQueue[idx].pop_front();
					}
				}
			}
			if (!co) {
				for (int i = 0; i < m_threadNum; i++) {
					ScopedLock _(m_lock[i]);
					if (!m_queue[i].empty()) {
						co = m_queue[i].front();
						m_queue[i].pop_front();
						break;
					}
				}
			}
			if (!co) {
				m_semaphore[idx].down();
				continue;
			}
			co->task()->associateThread(idx + 1);
			cs.resume(co);
			int status = co->status();
			switch (status) {
			case Coroutine::DEAD:
				Task::destory(co->task());
				delete co;
				break;
			case Coroutine::READY:
				Task::destory(co->task());
				freeCoroutine(co);
				break;
			case Coroutine::WAITING:
				break;
			case Coroutine::RUNNING:
			case Coroutine::SUSPEND:
			default:
				{
					ScopedLock _(m_lock[idx]);
					m_queue[idx].push_back(co);
				}
			}
		}
	}

	void Pool::s_routine(void* ptr) {
		static_cast<Pool*>(ptr)->routine();
	}

	Coroutine * Pool::allocCoroutine(size_t idx, ITask *task) {
		Coroutine * co = NULL;
		if (false)
		{
			ScopedLock _(m_freeLock);
			if (m_freeCount > 0) {
				co = m_freeQueue.front();
				m_freeQueue.pop_front();
				--m_freeCount;
				co->reset(task);
			}
		}
		if (!co) {
			co = new Coroutine(task);
		}
		task->associate(co);
		return co;
	}

	void Pool::freeCoroutine(Coroutine *co) {
		if (m_freeCount >= MAX_FREE_QUEUE_SIZE) {
			delete co;
			return;
		}
		{
			ScopedLock _(m_freeLock);
			m_freeQueue.push_back(co);
			++m_freeCount;
		}
	}

	void Pool::checkSemaphore(TaskQueueType &list, Semaphore &sem) {
		if (list.size() == 1) {
			sem.up();
		}
	}

	bool Pool::removeTask(ITask *task) {
		if (task && task->associateThread() == 0) {
			for (int i = 0; i < m_threadNum; i++) {
				ScopedLock _(m_lock[i]);
				for (TaskQueueType::iterator it = m_queue[i].begin(); it != m_queue[i].end(); ++it) {
					if ((*it)->task() == task) {
						if (task->associateThread()) {
							return false;
						}
						freeCoroutine(*it);
						m_queue[i].erase(it);
						delete task;
						return true;
					}
				}
			}
		}
		return false;
	}

	struct TriboolAnd
	{
		static tribool zero() {
			return tribool(true);
		}
		static tribool apply(tribool lhs, tribool rhs) {
			return lhs && rhs;
		}
	};

	struct TriboolOr
	{
		static tribool zero() {
			return tribool(false);
		}
		static tribool apply(tribool lhs, tribool rhs) {
			return lhs || rhs;
		}
	};

	template<typename Operator>
	class WaitMany0
		: public DeferredContextInterface
		, public DeferredData {
	public:
		void init(std::vector<IPromise>& promises) {
			m_promises.swap(promises);
			for (std::vector<IPromise>::iterator i = m_promises.begin(); i != m_promises.end(); ++i) {
				i->always(boost::bind(&WaitMany0::onAlways, this, boost::placeholders::_1));
			}
			WaitMany0::onAlways(true);
		}
		virtual void cancelMe() {
			for (std::vector<IPromise>::iterator i = m_promises.begin(); i != m_promises.end(); ++i) {
				i->cancel();
			}
		}
		virtual tribool resetMe() {
			tribool state = Operator::zero();
			for (std::vector<IPromise>::iterator i = m_promises.begin(); i != m_promises.end(); ++i) {
				state = Operator::apply(state, i->reset());
			}
			return state;
		}
		~WaitMany0() {
			for (std::vector<IPromise>::iterator i = m_promises.begin(); i != m_promises.end(); ++i) {
				i->removeAlways();
			}
		}
	protected:
		Mutex m_lock;

		virtual void done() {}
		virtual void onAlways(tribool) {
			tribool s = Operator::zero();
			for (std::vector<IPromise>::iterator i = m_promises.begin(); i != m_promises.end(); ++i) {
				s = Operator::apply(s, i->state());
			}
			{
				ScopedLock _(m_lock);
				if (indeterminate(state())) {

					if (s) {
						resolve();
						done();
					}
					else if (!s) {
						reject();
					}
					else {
						// nothing to do
					}
				}
			}
		}
	private:
		std::vector<IPromise> m_promises;
	};

	template<typename Operator>
	class WaitMany1 : public WaitMany0<Operator> {
	public:
		void init(std::vector<IPromise>& promises, unsigned int timeoutMs) {
			WaitMany0<Operator>::init(promises);
			m_timeout = createTimeout(timeoutMs);
			m_timeout.always(boost::bind(&WaitMany1::onAlways, this, boost::placeholders::_1));
			WaitMany1::onAlways(true);
		}
	protected:
		virtual void done() {
			m_timeout.cancel();
		}
		virtual void onAlways(tribool res) {
			WaitMany0<Operator>::onAlways(res);
			{
				ScopedLock _(WaitMany0<Operator>::m_lock);
				if (indeterminate(WaitMany0<Operator>::state())) {
					if (m_timeout.state()) {
						WaitMany0<Operator>::reject();
					}
				}
			}
		}
	private:
		Promise<void> m_timeout;
	};

	template<typename Operator>
	class WaitMany2 : public WaitMany1<Operator> {
	public:
		void init(std::vector<IPromise>& promises, unsigned int timeoutMs, CancelToken token) {
			WaitMany1<Operator>::init(promises, timeoutMs);
			m_cancel = token.promise();
			m_cancel.always(boost::bind(&WaitMany2::onAlways, this, boost::placeholders::_1));
			WaitMany2::onAlways(true);
		}
	protected:
		virtual void onAlways(tribool res) {
			WaitMany0<Operator>::onAlways(res);
			{
				ScopedLock _(WaitMany0<Operator>::m_lock);
				if (indeterminate(WaitMany0<Operator>::state())) {
					if (m_cancel.state()) {
						WaitMany0<Operator>::reject();
					}
				}
			}
		}
	private:
		Promise<void> m_cancel;
	};

	Promise<void> waitAll(std::vector<IPromise> promises, unsigned int timeoutMs, CancelToken token) {
		boost::shared_ptr<WaitMany2<TriboolAnd>> wait(new WaitMany2<TriboolAnd>());
		wait->init(promises, timeoutMs, token);
		return await(Promise<void>(wait));
	}
	Promise<void> waitAll(std::vector<IPromise> promises, unsigned int timeoutMs) {
		boost::shared_ptr<WaitMany1<TriboolAnd>> wait(new WaitMany1<TriboolAnd>());
		wait->init(promises, timeoutMs);
		return await(Promise<void>(wait));
	}
	Promise<void> waitAll(std::vector<IPromise> promises) {
		boost::shared_ptr<WaitMany0<TriboolAnd>> wait(new WaitMany0<TriboolAnd>());
		wait->init(promises);
		return await(Promise<void>(wait));
	}

	Promise<void> waitAny(std::vector<IPromise> promises, unsigned int timeoutMs, CancelToken token) {
		boost::shared_ptr<WaitMany2<TriboolOr>> wait(new WaitMany2<TriboolOr>());
		wait->init(promises, timeoutMs, token);
		return await(Promise<void>(wait));
	}
	Promise<void> waitAny(std::vector<IPromise> promises, unsigned int timeoutMs) {
		boost::shared_ptr<WaitMany1<TriboolOr>> wait(new WaitMany1<TriboolOr>());
		wait->init(promises, timeoutMs);
		return await(Promise<void>(wait));
	}
	Promise<void> waitAny(std::vector<IPromise> promises) {
		boost::shared_ptr<WaitMany0<TriboolOr>> wait(new WaitMany0<TriboolOr>());
		wait->init(promises);
		return await(Promise<void>(wait));
	}

	boost::asio::io_service m_service;
	boost::asio::io_service::work m_monitor(m_service);

	class TimerService {
		friend class Singleton<TimerService>;
		TimerService() : m_monitor(m_service), m_exit(false), m_thr(&TimerService::routine, this) {
		}
	public:
		~TimerService() {
			m_exit = true;
			m_service.stop();
			m_thr.join();
		}
		boost::asio::io_service& service() {
			return m_service;
		}
	private:
		boost::asio::io_service m_service;
		boost::asio::io_service::work m_monitor;
		void routine() {
			while (!m_exit) {
				m_service.run();
			}
		}
		static void routine(void* ctx) {
			reinterpret_cast<TimerService*>(ctx)->routine();
		}
		volatile bool m_exit;
		Thread m_thr;
	};

	class TimeoutPromise : public DeferredContext<void> {
	public:
		TimeoutPromise(unsigned int timeoutMs) : m_deadline(Singleton<TimerService>::inst().service()) {
			m_deadline.expires_from_now(boost::posix_time::milliseconds(timeoutMs));
			m_deadline.async_wait(boost::bind(&TimeoutPromise::OnTimeout, this, boost::placeholders::_1));
		}
		virtual void cancelMe() {
			m_deadline.cancel();
		}
	private:
		boost::asio::deadline_timer m_deadline;
		void OnTimeout(const boost::system::error_code& error) {
			if (error != boost::asio::error::operation_aborted) {
				resolve();
			} else {
				std::cout <<"cancel"<<std::endl;
			}
			delete this;
		}
	};

	Promise<void> createTimeout(unsigned int timeoutMs) {
		return (new TimeoutPromise(timeoutMs))->promise();
	}
}
