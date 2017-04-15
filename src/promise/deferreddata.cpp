#include <promise/detail/deferreddata.h>
#include <promise/detail/deferredbase.h>

namespace Task {

	typedef boost::shared_ptr<DeferredData> DeferredDataPtr;
	DeferredData::DeferredData() 
		: m_state(indeterminate), m_activating(false), m_ownContext(false)
		, m_context(NULL) {}
	DeferredData::DeferredData(tribool s) 
		: m_state(s), m_activating(false), m_ownContext(false) 
		, m_context(NULL) {}
	DeferredData::DeferredData(DeferredContextInterface *ctx) 
		: m_state(indeterminate), m_activating(false), m_ownContext(false) 
		, m_context(ctx) {}

	DeferredData::~DeferredData() {
		if (m_ownContext && m_context) {
			delete m_context;
		}
	}

	void DeferredData::resolve() {
		DeferredDataPtr hold(shared_from_this());
		m_state = true;
		m_activating = true;
		if (m_done) {
			m_done(m_state);
		}
		if (m_always) {
			m_always(m_state);
		}
		m_activating = false;
	}
	void DeferredData::reject() {
		DeferredDataPtr hold(shared_from_this());
		m_state = false;
		m_activating = true;
		if (m_fail) {
			m_fail(m_state);
		}
		if (m_always) {
			m_always(m_state);
		}
		m_activating = false;
	}
	void DeferredData::cancel() {
		m_state = false;
		if (m_context) {
			DeferredDataPtr hold(shared_from_this());
			m_context->cancelMe();
		}
	}
	tribool DeferredData::reset() {
		if (m_context) {
			DeferredDataPtr hold(shared_from_this());
			m_state = m_context->resetMe();
			if (indeterminate(m_state)) {
				m_result = boost::any();
			}
		}
		return m_state;
	}

	bool DeferredData::done(const PromiseCallback &func) {
		if (m_state) {
			func(m_state);
			return true;
		}
		if (m_done != NULL) {
			return false;
		}
		m_done = func;
		return true;
	}
	bool DeferredData::fail(const PromiseCallback &func) {
		if (m_state) {
			func(m_state);
			return true;
		}
		if (m_fail != NULL) {
			return false;
		}
		m_fail = func;
		return true;
	}
	bool DeferredData::always(const PromiseCallback &func) {
		if (m_state) {
			func(m_state);
			return true;
		}
		if (m_always != NULL) {
			return false;
		}
		m_always = func;
		return true;
	}

	void DeferredData::removeDone() {
		m_done = NULL;
	}
	void DeferredData::removeFail() {
		m_fail = NULL;
	}
	void DeferredData::removeAlways() {
		m_always = NULL;
	}

	DeferredContextInterface * DeferredData::context() const {
		return m_context;
	}

	void DeferredData::setContext(DeferredContextInterface *ctx) {
		m_context = ctx;
	}

	void DeferredData::setOwnContext(bool owned) {
		m_ownContext = owned;
	}

	bool DeferredData::ownContext() {
		return m_ownContext;
	}

	void DeferredData::setResult(const boost::any &r) {
		m_result = r;
	}

	const boost::any & DeferredData::result() {
		return m_result;
	}

	void DeferredData::resetState(tribool s) {
		m_state = s;
	}

	tribool DeferredData::state() const {
		return m_state;
	}

	bool DeferredData::isRejected() const {
		return !m_state;
	}

	bool DeferredData::isResolved() const {
		return m_state;
	}

	bool DeferredData::isActivating() const {
		return m_activating;
	}
}
