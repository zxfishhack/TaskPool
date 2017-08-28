include Rule.make

.PHONY: test
.PHONY: test_clean
.PHONY: tsakpool
.PHONY: taskpool_clean

all : taskpool test

distclean : clean
	-rm -rf bin
	-rm -rf objs
	-rm -rf lib

clean : test_clean taskpool_clean

test:
	$(MAKE) -fmakefile.mk -C$(PROJECT_ROOT_PATH)/tests build MODULE=test

test_clean:
	$(MAKE) -fmakefile.mk -C$(PROJECT_ROOT_PATH)/tests clean MODULE=test

taskpool:
	$(MAKE) -fmakefile.mk -C$(PROJECT_ROOT_PATH)/src build MODULE=taskpool

taskpool_clean:
	$(MAKE) -fmakefile.mk -C$(PROJECT_ROOT_PATH)/src clean MODULE=taskpool

