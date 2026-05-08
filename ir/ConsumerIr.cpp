/*
 * SPDX-FileCopyrightText: 2017-2024 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#define LOG_TAG "ConsumerIr"

#include <fcntl.h>
#include <linux/lirc.h>
#include <hardware/hardware.h>
#include <hardware/consumerir.h>
#include <android-base/logging.h>
#include <android-base/unique_fd.h>
#include <ir.sysprop.h>
#include "ConsumerIr.h"

using namespace ::vendor::lineage::ir;

static const char kLircDevice[] = "/dev/lirc0";
static const char kSpiDevice[] = "/dev/spidev7.1";

namespace aidl {
namespace android {
namespace hardware {
namespace ir {

ConsumerIr::ConsumerIr() {
    if (access(kLircDevice, F_OK) == 0) {
        mUseLirc = true;
        LOG(INFO) << "Using LIRC device " << kLircDevice;
    } else if (access(kSpiDevice, F_OK) == 0) {
        const hw_module_t* hw_module = nullptr;
        int ret = hw_get_module_by_class(CONSUMERIR_HARDWARE_MODULE_ID, "spi", &hw_module);
        if (ret != 0) {
            LOG(ERROR) << "hw_get_module failed for " << kSpiDevice << ": " << ret;
        } else {
            ret = hw_module->methods->open(hw_module, CONSUMERIR_TRANSMITTER,
                                           (hw_device_t**)&mSpiDevice);
            if (ret != 0) {
                LOG(ERROR) << "Failed to open " << kSpiDevice << ": " << ret;
            } else {
                mUseSpi = true;
                LOG(INFO) << "Using SPI device " << kSpiDevice;
            }
        }
    } else {
        LOG(ERROR) << "No IR device available";
    }

    auto carrier_freqs = IrProperties::carrier_freqs();
    if (carrier_freqs.size() >= 2) {
        for (size_t i = 0; i < carrier_freqs.size() - 1; i += 2) {
            if (!carrier_freqs[i] || !carrier_freqs[i + 1]) {
                continue;
            }
            kRangeVec.push_back({
                    .minHz = carrier_freqs[i].value(),
                    .maxHz = carrier_freqs[i + 1].value(),
            });
        }
    } else {
        kRangeVec.push_back({.minHz = 30000, .maxHz = 60000});
    }
}

::ndk::ScopedAStatus ConsumerIr::getCarrierFreqs(std::vector<ConsumerIrFreqRange>* _aidl_return) {
    *_aidl_return = kRangeVec;
    return ::ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus ConsumerIr::transmit(int32_t carrierFreqHz,
                                          const std::vector<int32_t>& pattern) {
    size_t entries = pattern.size();
    if (entries == 0) {
        return ::ndk::ScopedAStatus::ok();
    }

    if (!isInRange(carrierFreqHz)) {
        LOG(ERROR) << "Unsupported carrier " << carrierFreqHz;
        return ::ndk::ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);
    }

    if (entries % 2 == 0) {
        entries--;
    }

    if (mUseLirc) {
        ::android::base::unique_fd fd(open(kLircDevice, O_WRONLY));
        if (!fd.ok()) {
            LOG(ERROR) << "Failed to open " << kLircDevice << ": " << strerror(errno);
            return ::ndk::ScopedAStatus::fromExceptionCode(EX_ILLEGAL_STATE);
        }
        int rc = ioctl(fd.get(), LIRC_SET_SEND_CARRIER, &carrierFreqHz);
        if (rc < 0) {
            LOG(ERROR) << "Failed to set carrier " << carrierFreqHz << ": " << strerror(errno);
            return ::ndk::ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);
        }
        rc = write(fd.get(), pattern.data(), entries * sizeof(int32_t));
        if (rc < 0) {
            LOG(ERROR) << "Failed to write pattern, " << entries << " entries: " << strerror(errno);
            return ::ndk::ScopedAStatus::fromExceptionCode(EX_ILLEGAL_STATE);
        }
    } else if (mUseSpi) {
        int rc = mSpiDevice->transmit(mSpiDevice, carrierFreqHz, pattern.data(), entries);
        if (rc != 0) {
            LOG(ERROR) << "Failed to transmit pattern, error: " << rc;
            return ::ndk::ScopedAStatus::fromExceptionCode(EX_ILLEGAL_STATE);
        }
    } else {
        LOG(ERROR) << "No IR device available";
        return ::ndk::ScopedAStatus::fromExceptionCode(EX_ILLEGAL_STATE);
    }

    return ::ndk::ScopedAStatus::ok();
}

bool ConsumerIr::isInRange(int32_t carrierFreqHz) {
    for (const auto& range : kRangeVec) {
        if (carrierFreqHz >= range.minHz && carrierFreqHz <= range.maxHz) {
            return true;
        }
    }
    return false;
}

}  // namespace ir
}  // namespace hardware
}  // namespace android
}  // namespace aidl
