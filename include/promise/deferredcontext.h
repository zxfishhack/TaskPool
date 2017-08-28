#ifndef _TASK_POOL_DEFERRED_CONTEXT_H_
#define _TASK_POOL_DEFERRED_CONTEXT_H_

#include "promise.h"
#include "detail/deferreddata.h"

namespace Task {
	#define DEFERRED_OFFSETOF(t,m)   ((size_t)((char*)(&((t*)0)->m)))

	#define PINPOINT_DEFERRED_CONTEXT(derived, ctxtype, ctxname) \
		ctxtype* toContext() {return &this->ctxname;} \
		static derived* fromContext(ctxtype* ctx) {return (derived*)(((char*)ctx) - DEFERRED_OFFSETOF(derived, ctxname));}

	template<typename Ty>
	class DeferredContext
		: public DeferredBase
		, DeferredContextInterface {
	public:
		DeferredContext();
		virtual ~DeferredContext() = 0;

		Promise<Ty> promise();

		void resolve(const Ty& res);
		void reject();
		virtual tribool resetMe();
	};

	template<>
	class DeferredContext<void>
		: public DeferredBase
		, DeferredContextInterface {
	public:
		DeferredContext()
			: DeferredBase(this), DeferredContextInterface() {}
		virtual ~DeferredContext() {
			setContext(NULL);
		}

		Promise<void> promise() {
			return Promise<void>(d_ptr);
		}

		void resolve() {
			if (!indeterminate(d_ptr->state())) {
				return;
			}
			d_ptr->resolve();
		}

		void reject() {
			if (!indeterminate(d_ptr->state())) {
				return;
			}
			d_ptr->reject();
		}

		virtual tribool resetMe() {
			return d_ptr->state();
		}
	};

#include "detail/deferredcontext_impl.h"
}

#endif
