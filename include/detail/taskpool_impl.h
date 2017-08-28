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
		if (indeterminate(DeferredContext<Ret>::state())) {
			DeferredContext<Ret>::reject();
		}
	}
	void run() {
		Ret ret = m_func();
		if (indeterminate(DeferredContext<Ret>::state())) {
			DeferredContext<Ret>::resolve(ret);
		} else {
			//do nothing
		}
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
		if (indeterminate(DeferredContext<void>::state())) {
			DeferredContext<void>::reject();
		}
	}
	void run() {
		m_func();
		if (indeterminate(DeferredContext<void>::state())) {
			DeferredContext<void>::resolve();
		}
		else {
			//do nothing
		}
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

	friend Promise<void> waitAll(std::vector<IPromise> promises, unsigned int timeoutMs, CancelToken token);
	friend Promise<void> waitAny(std::vector<IPromise> promises, unsigned int timeoutMs, CancelToken token);
	template<typename Operator> friend class WaitMany2;
};

#endif
