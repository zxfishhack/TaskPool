#include <iostream>
#include <boost/bind.hpp>
#include <boost/function.hpp>

#include <taskpool.h>

#include <platform/sync.h>
#include <coroutine/task.h>



Task::Mutex g_outputLock;

boost::atomic<int> doneJob(0);

int TestAsync(int i) {
	//Task::await(Task::createTimeout(500 * i));
	return i * 10;
}

void FunctionTask(int ud) {
	std::vector<Task::IPromise> waitList;
	std::vector<Task::Promise<int>> resultList;
	for (int i = 0; i<ud; i++) {
		auto ret = Task::async(boost::bind(TestAsync, i));
		waitList.push_back(ret);
		resultList.push_back(ret);
	}
	auto waitResult = Task::waitAll(waitList);// , 5000);
	
	if (waitResult.isResolved()) {
		int sum = 0;
		for (auto i = resultList.begin(); i < resultList.end(); ++i) {
			sum += i->result();
		}
		//std::cout << "run sub taskes succ. val: " << sum << std::endl;
	}
	else {
		//std::cout << "run sub task failed." << std::endl;
	}
	{
		Task::ScopedLock _(g_outputLock);
		doneJob.fetch_add(1);
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
	Task::Pool tp(32);
	auto jobNum = 10000;
	auto submitJob = 0;
	for (int i = 0; i<jobNum; i++) {
		submitJob += tp.addTask(Task::Task::create(boost::bind(FunctionTask, 8 * ((i % 2) + 1)))) ? 1 : 0;
	}
	std::cout << "submit taskes succ." << std::endl;
	while(doneJob != submitJob) {
		Sleep(1);
	}
	std::cout << "all taskes done." << std::endl;
	tp.join();
	int c;
	std::cin >> c;
	return 0;
}