#!/usr/bin/env -S PYTHONPATH=../../../tools/extract-utils python3
#
# SPDX-FileCopyrightText: 2024 The LineageOS Project
# SPDX-License-Identifier: Apache-2.0
#

from extract_utils.fixups_blob import (
    blob_fixup,
    blob_fixups_user_type,
)
from extract_utils.fixups_lib import (
    lib_fixups,
    lib_fixups_user_type,
)
from extract_utils.main import (
    ExtractUtils,
    ExtractUtilsModule,
)

namespace_imports = [
    'device/xiaomi/sdm660-common',
    'hardware/qcom-caf/common/libqti-perfd-client',
    'hardware/qcom-caf/sdm660',
    'hardware/qcom-caf/wlan',
    'hardware/xiaomi',
    'vendor/qcom/opensource/commonsys/display',
    'vendor/qcom/opensource/commonsys-intf/display',
    'vendor/qcom/opensource/data-ipa-cfg-mgr-legacy-um',
    'vendor/qcom/opensource/dataservices',
    'vendor/qcom/opensource/display',
]

def lib_fixup_vendor_suffix(lib: str, partition: str, *args, **kwargs):
    return f'{lib}_{partition}' if partition == 'vendor' else None

lib_fixups: lib_fixups_user_type = {
    **lib_fixups,
    (
        'vendor/lib/libmmosal.so',
        'vendor/lib64/com.qualcomm.qti.dpm.api@1.0.so',
        'vendor/lib64/libmmosal.so',
        'vendor/lib64/vendor.qti.hardware.fm@1.0.so',
    ): lib_fixup_vendor_suffix,
}

blob_fixups: blob_fixups_user_type = {
    'vendor/bin/mlipayd@1.1': blob_fixup()
        .remove_needed('vendor.xiaomi.hardware.mtdservice@1.0.so'),
    ('vendor/lib64/libmlipay.so','vendor/lib64/libmlipay@1.1.so'): blob_fixup()
        .remove_needed('vendor.xiaomi.hardware.mtdservice@1.0.so')
        .binary_regex_replace(b'/system/etc/firmware', b'/vendor/firmware\x00\x00\x00\x00'),
    ('vendor/lib64/hw/consumerir.lirc.sdm660.so', 'consumerir.spi.sdm660.so'): blob_fixup()
        .fix_soname(),
    'vendor/bin/pm-service': blob_fixup()
        .add_needed('libutils-v33.so'),
    'vendor/lib64/libwvhidl.so': blob_fixup()
        .replace_needed('libcrypto.so', 'libcrypto-v33.so'),
}  # fmt: skip

module = ExtractUtilsModule(
    'sdm660-common',
    'xiaomi',
    blob_fixups=blob_fixups,
    lib_fixups=lib_fixups,
    namespace_imports=namespace_imports,
)

if __name__ == '__main__':
    utils = ExtractUtils.device(module)
    utils.run()
