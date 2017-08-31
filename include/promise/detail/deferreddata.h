#ifndef _TASK_POOL_DEFERRED_DATA_H_
#define _TASK_POOL_DEFERRED_DATA_H_

#include <global.h>
#include <boost/any.hpp>
#include <boost/function.hpp>
#include <boost/smart_ptr.hpp>

namespace Task 
{
	class DeferredContextInterface;

	class DeferredData : public boost::enable_shared_from_this<DeferredData> {
	public:
		typedef boost::shared_ptr<DeferredData> DeferredDataPtr;
		static DeferredDataPtr shared_indeterminate_deferred;
		static DeferredDataPtr shared_true_deferred;
		static DeferredDataPtr shared_false_deferred;

		typedef boost::function<void(tribool)> PromiseCallback;

		DeferredData();
		explicit DeferredData(tribool s);
		explicit DeferredData(DeferredContextInterface *ctx);
		virtual ~DeferredData();

		void resolve();
		void reject();
		void cancel();
		tribool reset();

		bool done(const PromiseCallback& func);
		bool fail(const PromiseCallback& func);
		bool always(const PromiseCallback& func);

		void removeDone();
		void removeFail();
		void removeAlways();

		DeferredContextInterface* context() const;
		void setContext(DeferredContextInterface* ctx);

		void setOwnContext(bool owned);
		bool ownContext();

		void setResult(const boost::any &r);
		const boost::any& result();

		void resetState(tribool s);
		tribool state() const;

		bool isRejected() const;
		bool isResolved() const;

		bool isActivating() const;
		boost::any m_result;
	protected:
		tribool m_state;
		bool m_activating;
		bool m_ownContext;
		

		DeferredContextInterface* m_context;
		PromiseCallback m_done;
		PromiseCallback m_fail;
		PromiseCallback m_always;

	};
}

#endif
