#ifndef _TASK_POOL_PROMISE_H_
#define _TASK_POOL_PROMISE_H_

#include <global.h>
#include <boost/smart_ptr.hpp>
#include "detail/deferredbase.h"
#include "detail/deferreddata.h"

namespace Task {

	typedef boost::shared_ptr<DeferredData> DeferredDataPtr;
	
	static DeferredDataPtr getSharedDeferred(tribool s) {
		if(s) {
			return DeferredData::shared_true_deferred;
		} else if(!s) {
			return DeferredData::shared_false_deferred;
		} else {
			return DeferredData::shared_indeterminate_deferred;
		}
	}
	
	class IPromise : public DeferredBase {
	public:
		IPromise() : DeferredBase(getSharedDeferred(indeterminate)) {}
		IPromise(bool s) : DeferredBase(getSharedDeferred(tribool(s))) {}
		explicit IPromise(DeferredDataPtr d) : DeferredBase(d) {}
		IPromise(const IPromise &rhs) : DeferredBase(rhs) {}
		
		~IPromise() {}
		
		tribool reset() {
			return d_ptr->reset();
		}
	};
	
	template<typename Ty = void>
	class Promise : public IPromise {
	public:
		Promise() {}
		Promise(bool s) : IPromise(s) {}
		Promise(const Promise &rhs) : IPromise(rhs) {}
		explicit Promise(DeferredDataPtr d) : IPromise(d) {}
		
		template<typename Other>
		Promise(const Promise<Other>& rhs) 
			: IPromise(rhs){
			BOOST_STATIC_ASSERT(boost::is_void<Ty>::value);
		}
		
		~Promise() {}
		
		Ty result() {
			return boost::any_cast<Ty>(d_ptr->result());
		}
		
		Promise& operator=(const Promise& rhs) {
			assign(rhs);
			return *this;
		}
	};
}

#endif
