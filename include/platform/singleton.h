#ifndef _TASK_POOL_SINGLETON_H_
#define _TASK_POOL_SINGLETON_H_
#include <boost/noncopyable.hpp>

namespace Task {
	template<typename T>
	class Singleton : public boost::noncopyable {
	public:
		static T& inst() {
			static T _inst;
			return _inst;
		}
	};
}

#endif
