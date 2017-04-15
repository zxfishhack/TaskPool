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
			resolve(ret);
		} else {
			//do nothing
		}
	}
private:
	boost::function<Ret()> m_func;
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
