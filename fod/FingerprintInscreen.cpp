/*
 * Copyright (C) 2019 The LineageOS Project
 * Copyright (C) 2020 The MoKee Open Source Project
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#define LOG_TAG "InscreenService"

#include "FingerprintInscreen.h"
#include "StellerClientCallback.h"
#include "KeyEventWatcher.h"

#include <android-base/logging.h>
#include <cutils/properties.h>
#include <hidl/HidlTransportSupport.h>
#include <fstream>
#include <cmath>
#include <thread>

#define NOTIFY_FINGER_DETECTED 1
#define NOTIFY_FINGER_REMOVED 2

#define BOOST_ENABLE_PATH "/sys/class/meizu/fp/qos_set"
#define HBM_ENABLE_PATH "/sys/class/meizu/lcm/display/hbm"
#define BRIGHTNESS_PATH "/sys/class/backlight/panel0-backlight/brightness"

#define FOD_POS_X 149 * 3
#define FOD_POS_Y 531 * 3
#define FOD_SIZE 62 * 3

#define TOUCHPANAL_DEV_PATH "/dev/input/event4"
#define KEY_FOD 0x0272

namespace vendor {
namespace mokee {
namespace biometrics {
namespace fingerprint {
namespace inscreen {
namespace V1_0 {
namespace implementation {

/*
 * Write value to path and close file.
 */
template <typename T>
static void set(const std::string& path, const T& value) {
    std::ofstream file(path);
    file << value;
}

template <typename T>
static T get(const std::string& path, const T& def) {
    std::ifstream file(path);
    T result;

    file >> result;
    return file.fail() ? def : result;
}

static KeyEventWatcher *keyEventWatcher;

static void sighandler(int) {
    LOG(INFO) << "Exiting";
    keyEventWatcher->exit();
}

FingerprintInscreen::FingerprintInscreen()
    : mFingerPressed{false}
    , mIconShown{false}
    {
    mSteller = ISteller::getService();
    mStellerClientCallback = new StellerClientCallback();

    keyEventWatcher = new KeyEventWatcher(TOUCHPANAL_DEV_PATH, [this](const std::string&, input_event evt) {
        if (evt.code == KEY_FOD) {
            notifyKeyEvent(evt.value);
        }
    });

    signal(SIGTERM, sighandler);
}

Return<int32_t> FingerprintInscreen::getPositionX() {
    return FOD_POS_X;
}

Return<int32_t> FingerprintInscreen::getPositionY() {
    return FOD_POS_Y;
}

Return<int32_t> FingerprintInscreen::getSize() {
    return FOD_SIZE;
}

Return<void> FingerprintInscreen::onStartEnroll() {
    return Void();
}

Return<void> FingerprintInscreen::onFinishEnroll() {
    return Void();
}

Return<void> FingerprintInscreen::onPress() {
    mFingerPressed = true;
    set(BOOST_ENABLE_PATH, 1);
    set(HBM_ENABLE_PATH, 1);
    std::thread([this]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(60));
        if (mFingerPressed) {
            notifyHal(NOTIFY_FINGER_DETECTED, 0);
        }
    }).detach();
    return Void();
}

Return<void> FingerprintInscreen::onRelease() {
    mFingerPressed = false;
    set(HBM_ENABLE_PATH, 0);
    notifyHal(NOTIFY_FINGER_REMOVED, 0);
    return Void();
}

Return<void> FingerprintInscreen::onShowFODView() {
    mIconShown = true;
    return Void();
}

Return<void> FingerprintInscreen::onHideFODView() {
    mIconShown = false;
    return Void();
}

Return<bool> FingerprintInscreen::handleAcquired(int32_t, int32_t) {
    return false;
}

Return<bool> FingerprintInscreen::handleError(int32_t, int32_t) {
    return false;
}

Return<void> FingerprintInscreen::setLongPressEnabled(bool) {
    return Void();
}

Return<int32_t> FingerprintInscreen::getDimAmount(int32_t) {
    int brightness = get(BRIGHTNESS_PATH, 0);
    float alpha = 1.0 - pow(brightness / 1023.0f, 0.455);
    float min = (float) property_get_int32("fod.dimming.min", 0);
    float max = (float) property_get_int32("fod.dimming.max", 255);
    return min + (max - min) * alpha;
}

Return<bool> FingerprintInscreen::shouldBoostBrightness() {
    return false;
}

Return<void> FingerprintInscreen::setCallback(const sp<IFingerprintInscreenCallback>& callback) {
    std::lock_guard<std::mutex> _lock(mCallbackLock);
    mCallback = callback;
    return Void();
}

void FingerprintInscreen::notifyHal(int32_t status, int32_t data) {
    Return<void> ret = this->mSteller->notify(status, data, mStellerClientCallback);
    if (!ret.isOk()) {
        LOG(ERROR) << "notifyHal(" << status << ") error: " << ret.description();
    }
}

void FingerprintInscreen::notifyKeyEvent(int value) {
    if (!mIconShown) {
        return;
    }

    LOG(INFO) << "notifyKeyEvent: " << value;

    std::lock_guard<std::mutex> _lock(mCallbackLock);
    if (mCallback == nullptr) {
        return;
    }

    if (value) {
        Return<void> ret = mCallback->onFingerDown();
        if (!ret.isOk()) {
            LOG(ERROR) << "FingerDown() error: " << ret.description();
        }
    } else {
        Return<void> ret = mCallback->onFingerUp();
        if (!ret.isOk()) {
            LOG(ERROR) << "FingerUp() error: " << ret.description();
        }
    }
}

}  // namespace implementation
}  // namespace V1_0
}  // namespace inscreen
}  // namespace fingerprint
}  // namespace biometrics
}  // namespace mokee
}  // namespace vendor
