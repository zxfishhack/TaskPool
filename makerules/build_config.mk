CFLAGS_GLOBAL_x86_debug   = -m32 -g -Wall -fPIC
CFLAGS_GLOBAL_x86_release = -m32 -O3 -Wall -fPIC
CFLAGS_GLOBAL_x64_debug   = -m64 -g -Wall -fPIC
CFLAGS_GLOBAL_x64_release = -m64 -O3 -Wall -fPIC

DEFINE_GLOBAL_x86_debug = -D_DEBUG
DEFINE_GLOBAL_x64_debug = -D_DEBUG

LIB_DIR_x86_debug   = lib/x86_debug
LIB_DIR_x86_release = lib/x86_release
LIB_DIR_x64_debug   = lib/x64_debug
LIB_DIR_x64_release = lib/x64_release
