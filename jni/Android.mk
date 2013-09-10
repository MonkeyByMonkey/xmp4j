LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

include $(LOCAL_PATH)/libxmp/src/Makefile
include $(LOCAL_PATH)/libxmp/src/depackers/Makefile
include $(LOCAL_PATH)/libxmp/src/loaders/Makefile
include $(LOCAL_PATH)/libxmp/src/loaders/prowizard/Makefile

SRC_SOURCES         := $(addprefix libxmp/src/,$(SRC_OBJS:.o=.c))
DEPACKERS_SOURCES   := $(addprefix libxmp/src/depackers/,$(DEPACKERS_OBJS:.o=.c))
LOADERS_SOURCES     := $(addprefix libxmp/src/loaders/,$(LOADERS_OBJS:.o=.c))
PROWIZ_SOURCES      := $(addprefix libxmp/src/loaders/prowizard/,$(PROWIZ_OBJS:.o=.c))

LOCAL_MODULE        := xmp4j
LOCAL_CFLAGS        := -I$(LOCAL_PATH)/libxmp/src -I$(LOCAL_PATH)/libxmp/include
LOCAL_SRC_FILES     := xmp4j.c $(SRC_SOURCES) $(DEPACKERS_SOURCES) $(LOADERS_SOURCES) $(PROWIZ_SOURCES)

ifeq ($(TARGET_ARCH_ABI),armeabi-v7a)
    LOCAL_ARM_NEON      := true
endif

include $(BUILD_SHARED_LIBRARY)