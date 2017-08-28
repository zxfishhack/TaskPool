#ifndef _TASK_POOL_DEFERRED_BASE_H_
#define _TASK_POOL_DEFERRED_BASE_H_

#include <global.h>
#include <boost/smart_ptr.hpp>
#include <boost/function.hpp>

namespace Task 
{
	class DeferredData;
	class DeferredContextInterface;
	typedef boost::shared_ptr<DeferredData> DeferredDataPtr;

	class DeferredBase {
	public:
#ifdef DEBUG
		char SIGNATURE[16];
#endif
		typedef boost::function<void(tribool)> PromiseCallback;

		DeferredBase();
		explicit DeferredBase(DeferredDataPtr d);
		explicit DeferredBase(DeferredContextInterface *ctx);
		DeferredBase(const DeferredBase &rhs);
		virtual ~DeferredBase();

		void cancel() const;

		bool done(const PromiseCallback& func) const;
		bool fail(const PromiseCallback& func) const;
		bool always(const PromiseCallback& func) const;

		void removeDone() const;
		void removeFail() const;
		void removeAlways() const;

		bool isRejected() const;
		bool isResolved() const;

		bool isActivating() const;

		tribool state() const;

		void setContext(DeferredContextInterface *ctx) const;
	protected:
		void assign(const DeferredBase& rhs);
		DeferredDataPtr d_ptr;
	};

	class DeferredContextInterface {
	public:
		DeferredContextInterface() {}
		virtual ~DeferredContextInterface() {}
		virtual void cancelMe() = 0;
		virtual tribool resetMe() = 0;
	};
}

#endif
