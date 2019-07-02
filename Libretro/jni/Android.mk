LOCAL_PATH := $(call my-dir)

ROOT_DIR     := $(LOCAL_PATH)/../..
CORE_DIR     := $(ROOT_DIR)/Core
LIBRETRO_DIR := $(ROOT_DIR)/Libretro
SEVENZIP_DIR := $(ROOT_DIR)/SevenZip
UTIL_DIR     := $(ROOT_DIR)/Utilities

HAVE_NETWORK := 1

include $(LIBRETRO_DIR)/Makefile.common

COREFLAGS := -DLIBRETRO

GIT_VERSION := " $(shell git rev-parse --short HEAD || echo unknown)"
ifneq ($(GIT_VERSION)," unknown")
  COREFLAGS += -DGIT_VERSION=\"$(GIT_VERSION)\"
endif

include $(CLEAR_VARS)
LOCAL_MODULE       := retro
LOCAL_SRC_FILES    := $(SOURCES_CXX) $(SOURCES_C)
LOCAL_CFLAGS       := $(COREFLAGS)
LOCAL_CXXFLAGS     := $(COREFLAGS) -std=c++11
LOCAL_LDFLAGS      := -Wl,-version-script=$(LIBRETRO_DIR)/link.T
LOCAL_CPP_FEATURES := exceptions rtti
include $(BUILD_SHARED_LIBRARY)
