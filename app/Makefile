#############################################################
# Required variables for each makefile
# Discard this section from all parent makefiles
# Expected variables (with automatic defaults):
#   CSRCS (all "C" files in the dir)
#   SUBDIRS (all subdirs with a Makefile)
#   GEN_LIBS - list of libs to be generated ()
#   GEN_IMAGES - list of object file images to be generated ()
#   GEN_BINS - list of binaries to be generated ()
#   COMPONENTS_xxx - a list of libs/objs in the form
#     subdir/lib to be extracted and rolled up into
#     a generated lib/image xxx.a ()
#					bkys注释于2017年10月9日
TARGET = eagle
#FLAVOR = release
FLAVOR = debug

#EXTRA_CCFLAGS += -u

#如果没有定义PDIR，现在还没有定义所以执行里面内容
ifndef PDIR # {
GEN_IMAGES= eagle.app.v6.out
GEN_BINS= eagle.app.v6.bin
#APP_MKTARGETS设定编译目标
SPECIAL_MKTARGETS=$(APP_MKTARGETS)
SUBDIRS=    \
	user    \
	mqtt   \
	modules  

endif # } PDIR

APPDIR = .
LDDIR = ../ld

#-Os是一个额外的编译标志，告诉GCC优先优化尺寸.
#	/Os（代码大小优先）通过指示编译器优选大小而非速度来最小化 EXE 和 DLL 的大小。编译器可以将许多 C 和 C++ 构造缩小为功能类似的机器码序列。这些差异有时在大小和速度之间提供了折中。/Os 和 /Ot 选项允许在二者之间指定一个首选项：
#  	/Ot（代码速度优先）通过指示编译器优选速度而非大小来最大化 EXE 和 DLL 的速度。（这是默认设置。）编译器可以将许多 C 和 C++ 构造缩小为功能类似的机器码序列。这些差异有时在大小和速度之间提供了折衷。“最大化速度”(/O2/O1、/O2（最小化大小、最大化速度）) 选项隐含 /Ot 选项。/O2 选项组合若干个选项以产生速度非常快的代码。
#	如果使用 /Os 或 /Ot，还必须指定 /Og/Og（全局优化）以优化代码。
#
#
CCFLAGS += -Os

TARGET_LDFLAGS =		\
	-nostdlib		\
	-Wl,-EL \
	--longcalls \
	--text-section-literals
#调试模式还是发行模式，如果是调试模式则。-O2与-O0这两者的差异
ifeq ($(FLAVOR),debug)
    TARGET_LDFLAGS += -g -O2
endif
#调试模式还是发行模式，如果是发布模式则。-O2与-O0这两者的差异
ifeq ($(FLAVOR),release)
    TARGET_LDFLAGS += -g -O0
endif

#编译器目标标识，要生成什么样的结果，在这里是libuser.a、libmqtt.a、libmodules.a
COMPONENTS_eagle.app.v6 = \
	user/libuser.a  \
	mqtt/libmqtt.a  \
	modules/libmodules.a
#连接标识
LINKFLAGS_eagle.app.v6 = \
	-L../lib        \			 #定义链接库的搜索路径是~/Share/SDK/lib/
	-nostdlib	\				#不连接系统标准启动文件和标准库文件，只把指定的文件传递给连接器。
    -T$(LD_FILE)   \			#-T 文件, --script 文件  读取链接脚本 
	-Wl,--no-check-sections	\	#Do not check section addresses for overlaps  不检查重叠地址
	-Wl,--gc-sections	\		 #减少静态库不必要的调用  -Wl,<选项>  将逗号分隔的 <选项> 传递给链接器
    -u call_user_start	\
	-Wl,-static						\		#为了确保使用的是静态库，则使用此选项。  优先使用静态库。选择其中用到的函数，节省空间
	-Wl,--start-group					\	 #库列表开始， 这些库最后会存在~\share\SDK\lib下
	-lc					\
	-lgcc					\			#libgcc.a
	-lhal					\
	-lphy	\
	-lpp	\
	-lnet80211	\
	-llwip	\
	-lwpa	\
	-lcrypto	\
	-lmain	\
	-ljson	\
	-lupgrade\
	-lssl	\
	-lpwm	\
	-ldriver \
	-lsmartconfig \
	$(DEP_LIBS_eagle.app.v6)					\	#在下面的内容有定义DEPENDS_eagle.app.v6
	-Wl,--end-group									#和上面的这个和 -Wl,--start-group一起使用

DEPENDS_eagle.app.v6 = \
                $(LD_FILE) \
                $(LDDIR)/eagle.rom.addr.v6.ld

#############################################################
# Configuration i.e. compile options etc.
# Target specific stuff (defines etc.) goes in here!
# Generally values applying to a tree are captured in the
#   makefile at its root level - these are then overridden
#   for a subtree within the makefile rooted therein
#

#UNIVERSAL_TARGET_DEFINES =		\

# Other potential configuration flags include:
#	-DTXRX_TXBUF_DEBUG
#	-DTXRX_RXBUF_DEBUG
#	-DWLAN_CONFIG_CCX
CONFIGURATION_DEFINES =	-DICACHE_FLASH \
                        -DGLOBAL_DEBUG_ON

DEFINES +=				\
	$(UNIVERSAL_TARGET_DEFINES)	\
	$(CONFIGURATION_DEFINES)

DDEFINES +=				\
	$(UNIVERSAL_TARGET_DEFINES)	\
	$(CONFIGURATION_DEFINES)


#############################################################
# Recursion Magic - Don't touch this!!
#
# Each subtree potentially has an include directory
#   corresponding to the common APIs applicable to modules
#   rooted at that subtree. Accordingly, the INCLUDE PATH
#   of a module can only contain the include directories up
#   its parent path, and not its siblings
#
# Required for each makefile to inherit from the parent
#

INCLUDES := $(INCLUDES) -I $(PDIR)include -I $(PDIR)include/mqtt -I $(PDIR)include/modules
PDIR := ../$(PDIR)
sinclude $(PDIR)Makefile			#首目录Makefile

.PHONY: FORCE
FORCE:

