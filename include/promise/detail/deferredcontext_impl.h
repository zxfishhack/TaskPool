#ifndef _TASK_POOL_DEFERRED_CONTEXT_IMPL_H_
#define _TASK_POOL_DEFERRED_CONTEXT_IMPL_H_

template<typename Ty>
DeferredContext<Ty>::DeferredContext() 
	: DeferredBase(this), DeferredContextInterface() 
{}

template<typename Ty>
DeferredContext<Ty>::~DeferredContext() {
	this->setContext(NULL);
}

template<typename Ty>
Promise<Ty> DeferredContext<Ty>::promise() {
	return Promise<Ty>(d_ptr);
}

template<typename Ty>
void DeferredContext<Ty>::resolve(const Ty& res) {
	if (indeterminate(d_ptr->state())) {
		d_ptr->setResult(res);
		d_ptr->resolve();
	}
}
template<typename Ty>
void DeferredContext<Ty>::reject() {
	if (indeterminate(d_ptr->state())) {
		d_ptr->reject();
	}
}
template<typename Ty>
tribool DeferredContext<Ty>::resetMe() {
	return d_ptr->state();
}

#endif
