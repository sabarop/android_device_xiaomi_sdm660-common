#
# Copyright (C) 2020-2024 The LineageOS Project
#
# SPDX-License-Identifier: Apache-2.0
#

LOCAL_PATH := $(call my-dir)

ifneq ($(filter tulip jasmine_sprout wayne clover lavender platina jason whyred,$(TARGET_DEVICE)),)

LPFLASH := $(HOST_OUT_EXECUTABLES)/lpflash$(HOST_EXECUTABLE_SUFFIX)
INSTALLED_SUPERIMAGE_DUMMY_TARGET := $(PRODUCT_OUT)/super_dummy.img

$(INSTALLED_SUPERIMAGE_DUMMY_TARGET): $(PRODUCT_OUT)/super_empty.img $(LPFLASH)
	$(call pretty,"Target dummy super image: $@")
	$(hide) touch $@
	$(hide) echo $(CURDIR)
	$(hide) $(LPFLASH) $@ $(PRODUCT_OUT)/super_empty.img

.PHONY: super_dummyimage
super_dummyimage: $(INSTALLED_SUPERIMAGE_DUMMY_TARGET)

INSTALLED_RADIOIMAGE_TARGET += $(INSTALLED_SUPERIMAGE_DUMMY_TARGET)

include $(CLEAR_VARS)

METADATA_SYMLINKS := $(TARGET_ROOT_OUT)/metadata
$(METADATA_SYMLINKS): $(LOCAL_INSTALLED_MODULE)
	@echo "Creating $@"
	@mkdir -p $(TARGET_ROOT_OUT)/metadata
	$(hide) ln -sf /data/apex $@/apex

ALL_DEFAULT_INSTALLED_MODULES += $(METADATA_SYMLINKS)

subdir_makefiles=$(call first-makefiles-under,$(LOCAL_PATH))
$(foreach mk,$(subdir_makefiles),$(info including $(mk) ...)$(eval include $(mk)))
endif
