#include <global.h>
#include <asio/asio.h>
#include <promise/deferredcontext.h>
#include <boost/smart_ptr.hpp>

namespace Task {

	struct CompleteData : public DeferredContext<Size> {
		OVERLAPPED overlap;
		boost::shared_array<char> bufHold;
		
		PINPOINT_DEFERRED_CONTEXT(CompleteData, OVERLAPPED, overlap)
		
		CompleteData(File file, Offset offset, boost::shared_array<char> buf) 
			: bufHold(buf) {
			LARGE_INTEGER li;
			li.QuadPart = offset;
			overlap.hEvent = file;
			overlap.Offset = li.LowPart;
			overlap.OffsetHigh = li.HighPart;
		}
		
		virtual void cancelMe() {
			HANDLE hFile = this->overlap.hEvent;
			::CancelIoEx(hFile, &this->overlap);
			overlap.hEvent = NULL;
		}
		
		static VOID CompleteRoutine(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED olp) {
			CompleteData *crData = CompleteData::fromContext(olp);
			if(0 == dwErrorCode || ERROR_HANDLE_EOF == dwErrorCode) {
				crData->resolve(dwNumberOfBytesTransfered);
			} else {
				crData->reject();
			}
			delete crData;
		}
	};
	
	namespace asio {
		File OpenFile(const std::string& name, std::ios::openmode mode) {
			DWORD dwDesiredAccess = 0;
			DWORD dwShareMode = 0;
			DWORD dwCreationDisposition = 0;
			if(mode & std::ios::in) {
				dwDesiredAccess |= GENERIC_READ;
			}
			if(mode & std::ios::out) {
				dwDesiredAccess |= GENERIC_WRITE;
			} else {
				dwShareMode |= FILE_SHARE_READ;
			}
			if(mode & std::ios::trunc) {
				dwCreationDisposition = TRUNCATE_EXISTING;
			}
			if(mode & std::ios::_Nocreate) {
				dwCreationDisposition |= OPEN_EXISTING;
			} else if(mode & std::ios::_Noreplace) {
				dwCreationDisposition |= CREATE_NEW;
			} else if(mode & std::ios::ate) {
				dwCreationDisposition |= OPEN_EXISTING;
			} else if(mode & std::ios::app) {
				dwCreationDisposition |= CREATE_NEW;
			} else {
				if (mode & std::ios::in) {
					dwCreationDisposition |= OPEN_EXISTING;
				} else {
					dwCreationDisposition |= CREATE_ALWAYS;
				}
			}
			File f = ::CreateFileA(name.c_str(), dwDesiredAccess, dwShareMode, NULL, dwCreationDisposition, FILE_FLAG_OVERLAPPED, NULL);
			if((mode & std::ios::ate) | (mode & std::ios::app)) {
				::SetFilePointer(f, 0, 0, FILE_END);
			}
			return f;
		}
		
		Promise<Size> asio::ReadFile(File file, Offset offset, Size size, boost::shared_array<char> buf) {
			CompleteData *crData = new CompleteData(file, offset, buf);
			if(ReadFileEx(file, buf.get(), size, crData->toContext(), &CompleteData::CompleteRoutine)) {
				return crData->promise();
			}
			delete crData;
			return false;
		}
			
		Promise<Size> asio::WriteFile(File file, Offset offset, Size size, boost::shared_array<char> buf) {
			CompleteData *crData = new CompleteData(file, offset, buf);
			if(WriteFileEx(file, buf.get(), size, crData->toContext(), &CompleteData::CompleteRoutine)) {
				return crData->promise();
			}
			delete crData;
			return false;
		}
	}
}