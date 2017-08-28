include $(MODULE).pro

include $(PROJECT_ROOT_PATH)/makerules/common_header.mk

# local flag define
INCLUDE += -I$(PROJECT_ROOT_PATH)/include -I$(PROJECT_ROOT_PATH)/vendor
CXX_OPTS += -std=c++11
CC_OPTS += -std=c++11
OPTI_OPTS +=
DEFINE +=
LD_OPTS += 
AR_OPTS +=

include $(PROJECT_ROOT_PATH)/makerules/common_footer.mk

build: 
	@echo \# Building $(MODULE)...
	$(MAKE) -fmakefile.mk libs MODULE=$(MODULE) TARGET=depend
	$(MAKE) -fmakefile.mk libs MODULE=$(MODULE)
	$(MAKE) -fmakefile.mk libs MODULE=$(MODULE) TARGET=lib

