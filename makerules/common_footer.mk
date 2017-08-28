ifndef $(COMMON_FOOTER_MK)
COMMON_FOORTER_MK = 1

OBJS_=$(subst .cpp,.o, $(SOURCES))
OBJS__=$(subst .c,.co, $(OBJS_))
OBJS=$(subst ../,,$(OBJS__))

OBJ_PATH=$(subst ../,,$(DEPENDPATH))

OBJ_DIR=$(OBJ_BASE_DIR)/$(MODULE)

LIB = lib$(MODULE).a

vpath %.o $(OBJ_DIR)

EXE = $(EXE_DIR)/$(MODULE)

SO_LIB = $(LIB_DIR)/lib$(MODULE).so

.cpp.o:
	@echo \# $(MODULE): $(PLATFORM): Compiling $<
	$(CXX) -c $(CXX_OPTS) $(OPTI_OPTS) $(DEFINE) $(INCLUDE) -o$(OBJ_DIR)/$@ $<

%.co: %.c
	@echo \# $(MODULE): $(PLATFORM): Compiling $<
	$(CC) -c $(CC_OPTS) $(OPTI_OPTS) $(DEFINE) $(INCLUDE) -o$(OBJ_DIR)/$@ $<

obj: $(OBJS)
	@echo \# $(MODULE): $(PLATFORM): end build objects.

clean:
	@echo \# $(MODULE): $(PLATFORM): Deleting temporary files
	-rm -f $(LIB_DIR)/$(LIB)
	-rm -f $(OBJ_DIR)/*.*
	-rm -f MAKEFILE_$(PLATFORM).DEPEND

MKDEP = $(CXX) $(CXX_OPTS) $(DEFINE) $(INCLUDE) -MM -MT _f_u_c_k_

depend: $(SOURCES)
#	@echo \# $(MODULE): $(PLATFORM): Making Directories, if not already created
	-mkdir -p $(LIB_DIR)
	-mkdir -p $(OBJ_DIR)
	-mkdir -p $(EXE_DIR)
	-cd $(OBJ_DIR); mkdir -p $(OBJ_PATH)
	-rm -f MAKEFILE_$(PLATFORM).DEPEND
	@echo \# $(MODULE): $(PLATFORM): Building dependancies
	@for file in $(SOURCES); do\
		$(MKDEP) $$file > .tmp; \
		$(PROJECT_ROOT_PATH)/makerules/swap.sh $$file .tmp >> MAKEFILE_$(PLATFORM).DEPEND; \
		rm .tmp; \
	done
#	$(MKDEP)

so:
	@echo \# $(MODULE): $(PLATFORM): Linking to .so
	cd $(OBJ_DIR) ; $(LD) $(OBJS) $(LD_OPTS) -fPIC -shared -o $(SO_LIB)
	@echo \#

lib: $(LIB_DIR)/$(LIB)

$(LIB_DIR)/$(LIB) : 
	@echo \# $(MODULE): $(PLATFORM): Creating archive $(LIB)
	cd $(OBJ_DIR) ; $(AR) $(AR_OPTS) $(LIB_DIR)/$(LIB) $(OBJS)
	@echo \# 

exe: 
	@echo \# $(MODULE): $(PLATFORM): Linking
	cd $(OBJ_DIR) ; $(LD) $(OBJS) $(LD_OPTS) -o$(EXE)
	@echo \#

libs:
	$(MAKE) -fmakefile.mk MODULE=$(MODULE) $(TARGET)

all: build

install:

-include MAKEFILE_$(PLATFORM).DEPEND

endif
