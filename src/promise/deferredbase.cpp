#include <promise/detail/deferredbase.h>
#include <promise/detail/deferreddata.h>

namespace Task {
	//static
	DeferredDataPtr DeferredData::shared_indeterminate_deferred(new DeferredData(indeterminate));
	//static
	DeferredDataPtr DeferredData::shared_true_deferred(new DeferredData(tribool(true)));
	//static
	DeferredDataPtr DeferredData::shared_false_deferred(new DeferredData(tribool(false)));

	DeferredBase::DeferredBase() 
		: d_ptr(new DeferredData())
	{}
	DeferredBase::DeferredBase(DeferredDataPtr d)
		: d_ptr(d)
	{}
	DeferredBase::DeferredBase(DeferredContextInterface *ctx)
		: d_ptr(new DeferredData(ctx)) 
	{}
	DeferredBase::DeferredBase(const DeferredBase &rhs) 
		: d_ptr(rhs.d_ptr) 
	{}
	DeferredBase::~DeferredBase() {}

	void DeferredBase::cancel() const {
		return d_ptr->cancel();
	}

	bool DeferredBase::done(const PromiseCallback &func) const {
		return d_ptr->done(func);
	}

	bool DeferredBase::fail(const PromiseCallback &func) const {
		return d_ptr->fail(func);
	}
	bool DeferredBase::always(const PromiseCallback &func) const {
		return d_ptr->always(func);
	}

	void DeferredBase::removeDone() const {
		return d_ptr->removeDone();
	}
	void DeferredBase::removeFail() const {
		return d_ptr->removeFail();
	}
	void DeferredBase::removeAlways() const {
		return d_ptr->removeAlways();
	}

	bool DeferredBase::isRejected() const {
		return d_ptr->isRejected();
	}
	bool DeferredBase::isResolved() const {
		return d_ptr->isResolved();
	}

	bool DeferredBase::isActivating() const {
		return d_ptr->isActivating();
	}

	tribool DeferredBase::state() const {
		return d_ptr->state();
	}

	void DeferredBase::setContext(DeferredContextInterface *ctx) const {
		d_ptr->setContext(ctx);
	}

	void DeferredBase::assign(const DeferredBase &rhs) {
		d_ptr = rhs.d_ptr;
	}
}
