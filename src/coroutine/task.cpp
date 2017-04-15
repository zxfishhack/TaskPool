#include <global.h>
#include <coroutine/task.h>
#include <coroutine/coroutine.h>
#include <taskpool.h>

namespace Task {

	void ITask::Run() {
		run();
	}

	bool ITask::cancelTask() {
		return internal::curPool.get()->cancelTask(m_cancelHandle);
	}

	void ITask::associate(Coroutine* co) {
		m_co = co;
	}

	Coroutine* ITask::associate() const {
		return m_co;
	}

	void ITask::associateThread(size_t thrId) {
		m_thrId = thrId;
	}

	size_t ITask::associateThread() const {
		return m_thrId;
	}

	void ITask::setCancelHandle(size_t cancelHandle) {
		m_cancelHandle = cancelHandle;
	}

	size_t ITask::cancelHandle() const {
		return m_cancelHandle;
	}

	ITask* Task::create(boost::function<void()> func) {
		return new Task(func);
	}

	void Task::destory(ITask *task) {
		delete task;
	}

	bool PromiseNotify::isCancelled() {
		return false;
	}

	void PromiseNotify::notify(tribool res) {
		assert(m_taskToNotify);
		m_pool->wakeupCoroutine(m_taskToNotify);
		m_taskToNotify = NULL;
	}

}
