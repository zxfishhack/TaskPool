MYSQL_CONNECTOR_BASE := $(PROJECT_ROOT_PATH)/../3rdparty/mysqlconnector

# debug_x86
ifeq ($(PLATFORM), x86_debug)
	MYSQLCONNECTOR_PATH := $(MYSQL_CONNECTOR_BASE)/v32_linux
endif

ifeq ($(PLATFORM), x86_release)
	MYSQLCONNECTOR_PATH := $(MYSQL_CONNECTOR_BASE)/v32_linux
endif

ifeq ($(PLATFORM), x64_debug)
	MYSQLCONNECTOR_PATH := $(MYSQL_CONNECTOR_BASE)/v64_linux
endif

ifeq ($(PLATFORM), x64_release)
	MYSQLCONNECTOR_PATH := $(MYSQL_CONNECTOR_BASE)/v64_linux
endif
