#ifndef _TASK_POOL_ASIO_H_
#define _TASK_POOL_ASIO_H_

#include <global.h>
#include <promise/promise.h>
#include <boost/shared_array.hpp>
#include <string>

namespace Task{

	typedef HANDLE File;
	const File INVALID_FILE = NULL;
	typedef LONGLONG Offset;
	typedef unsigned int Size;
	
	namespace asio {
		File OpenFile(const std::string& name, std::ios::openmode mode = std::ios::in | std::ios::binary);
		Promise<Size> ReadFile(File file, Offset offset, Size size, boost::shared_array<char> buf);
		Promise<Size> WriteFile(File file, Offset offset, Size size, boost::shared_array<char> buf);
	}
}
#endif
