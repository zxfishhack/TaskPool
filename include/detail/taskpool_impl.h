#ifndef _TASK_POOL_TASK_POOL_IMPL_H_
#define _TASK_POOL_TASK_POOL_IMPL_H_

template<typename Ret>
class PromiseTask 
	: public DeferredContext<Ret>
	, public ITask {
public:
	PromiseTask(boost::function<Ret()> func) 
		: m_func(func){}

	void cancelMe() {
		DeferredContext<Ret>::reject();
	}
	void run() {
		DeferredContext<Ret>::resolve(m_func());
	}
private:
	boost::function<Ret()> m_func;
};

template<>
class PromiseTask<void>
	: public DeferredContext<void>
	, public ITask {
public:
	PromiseTask(boost::function<void()> func)
		: m_func(func) {}

	void cancelMe() {
		DeferredContext<void>::reject();
	}
	void run() {
		m_func();
		DeferredContext<void>::resolve();
	}
private:
	boost::function<void()> m_func;
};

class CancelToken : protected DeferredContext<void> {
public:
	void apply() {
		resolve();
	}
private:
	void cancelMe(){}

	friend void waitAll(std::vector<IPromise> promises, unsigned int timeoutMs, CancelToken token);
	friend void waitAny(std::vector<IPromise> promises, unsigned int timeoutMs, CancelToken token);
	template<typename Operator> friend class WaitMany2;
};

#endif
