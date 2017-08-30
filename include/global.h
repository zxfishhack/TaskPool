#ifndef _TASK_POOL_GLOBAL_H_
#define _TASK_POOL_GLOBAL_H_

#define BOOST_ALL_NO_LIB

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif
#include <boost/logic/tribool.hpp>
#include <boost/static_assert.hpp>
#include <boost/noncopyable.hpp>
#include <exception>

using boost::logic::tribool;
using boost::logic::indeterminate;

const int STACK_SIZE = 1024;

#endif
