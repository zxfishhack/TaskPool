#include <iostream>
#include <boost/bind.hpp>
#include <boost/function.hpp>

#include <taskpool.h>

#include <platform/sync.h>
#include <coroutine/task.h>



Task::Mutex g_outputLock;

int TestAsync(int i) {
	Task::await(Task::createTimeout(500 * i));
	return i * 10;
}

void FunctionTask(int ud) {
	std::vector<Task::IPromise> waitList;
	std::vector<Task::Promise<int>> resultList;
	for(int i=0; i<ud; i++) {	
		auto ret = Task::async(boost::bind(TestAsync, i));
		waitList.push_back(ret);
		resultList.push_back(ret);
	}
	auto waitResult = Task::waitAll(waitList, 5000);
	Task::ScopedLock _(g_outputLock);
	if (waitResult.isResolved()) {
		int sum = 0;
		for (auto i = resultList.begin(); i < resultList.end(); ++i) {
			sum += i->result();
		}
		std::cout << "run sub taskes succ. val: " << sum << std::endl;
	} else {
		std::cout << "run sub task failed." << std::endl;
	}
}

int TestAsync2(int i) {
	return i * 10;
}

void FunctionTask2(int ud) {
	auto res = Task::await(Task::async(boost::bind(TestAsync2, ud))).result();
	std::cout << "result: " << res << std::endl;
}

int main() {
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//_CrtSetAllocHook(YourAllocHook);
	Task::Pool tp(32);
	for(int i=0; i<100; i++) {
		tp.addTask(Task::Task::create(boost::bind(FunctionTask, 8 * ((i % 2) + 1))));
	}
	tp.addTask(Task::Task::create(boost::bind(FunctionTask, 16)));
	int c;
	std::cin >> c;
	tp.join();
	_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
	return 0;
}