APP_STL:=c++_static
APP_ABI := all

# Don't strip debug builds
ifeq ($(NDK_DEBUG),1)
    cmd-strip := 
endif
