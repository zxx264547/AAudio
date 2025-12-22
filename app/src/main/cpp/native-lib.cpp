#include <jni.h>
#include <aaudio/AAudio.h>
#include <android/log.h>

#include <atomic>
#include <cmath>
#include <mutex>
#include <thread>
#include <chrono>

#define LOG_TAG "AAudioDemo"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

class NativeEngine {
public:
    void start();
    void stop();
    void release();

private:
    static aaudio_data_callback_result_t dataCallback(
            AAudioStream *stream,
            void *userData,
            void *audioData,
            int32_t numFrames);

    static void errorCallback(
            AAudioStream *stream,
            void *userData,
            aaudio_result_t error);

    bool openStream();
    void closeStream();
    void startStream();
    void restartLoop();

    std::mutex streamMutex;
    AAudioStream *stream = nullptr;

    std::atomic<bool> isPlaying{false};
    std::atomic<bool> restartRequested{false};
    std::atomic<bool> threadRunning{false};

    std::thread restartThread;
    std::atomic<double> phaseIncrement{0.0};
    double phase = 0.0;
};

void NativeEngine::start() {
    bool expected = false;
    if (!isPlaying.compare_exchange_strong(expected, true)) {
        return;
    }

    restartRequested.store(false);

    if (!openStream()) {
        isPlaying.store(false);
        return;
    }

    startStream();

    threadRunning.store(true);
    restartThread = std::thread(&NativeEngine::restartLoop, this);
}

void NativeEngine::stop() {
    isPlaying.store(false);
    restartRequested.store(false);

    threadRunning.store(false);
    if (restartThread.joinable()) {
        restartThread.join();
    }

    closeStream();
}

void NativeEngine::release() {
    stop();
}

bool NativeEngine::openStream() {
    AAudioStreamBuilder *builder = nullptr;
    aaudio_result_t result = AAudio_createStreamBuilder(&builder);
    if (result != AAUDIO_OK || builder == nullptr) {
        LOGE("AAudio_createStreamBuilder failed: %d", result);
        return false;
    }

    AAudioStreamBuilder_setDirection(builder, AAUDIO_DIRECTION_OUTPUT);
    AAudioStreamBuilder_setSharingMode(builder, AAUDIO_SHARING_MODE_SHARED);
    AAudioStreamBuilder_setPerformanceMode(builder, AAUDIO_PERFORMANCE_MODE_LOW_LATENCY);
    AAudioStreamBuilder_setFormat(builder, AAUDIO_FORMAT_PCM_FLOAT);
    AAudioStreamBuilder_setChannelCount(builder, 1);
    AAudioStreamBuilder_setSampleRate(builder, 48000);
    AAudioStreamBuilder_setDataCallback(builder, dataCallback, this);
    AAudioStreamBuilder_setErrorCallback(builder, errorCallback, this);

    {
        std::lock_guard<std::mutex> lock(streamMutex);
        result = AAudioStreamBuilder_openStream(builder, &stream);
    }

    AAudioStreamBuilder_delete(builder);

    if (result != AAUDIO_OK || stream == nullptr) {
        LOGE("AAudioStreamBuilder_openStream failed: %d", result);
        return false;
    }

    int32_t sampleRate = AAudioStream_getSampleRate(stream);
    const double kTwoPi = 6.2831853071795864769;
    phaseIncrement.store(kTwoPi * 440.0 / static_cast<double>(sampleRate));
    phase = 0.0;

    return true;
}

void NativeEngine::startStream() {
    std::lock_guard<std::mutex> lock(streamMutex);
    if (!stream) {
        return;
    }
    aaudio_result_t result = AAudioStream_requestStart(stream);
    if (result != AAUDIO_OK) {
        LOGE("AAudioStream_requestStart failed: %d", result);
    }
}

void NativeEngine::closeStream() {
    std::lock_guard<std::mutex> lock(streamMutex);
    if (!stream) {
        return;
    }

    AAudioStream_requestStop(stream);
    AAudioStream_close(stream);
    stream = nullptr;
}

void NativeEngine::restartLoop() {
    while (threadRunning.load()) {
        if (!isPlaying.load()) {
            break;
        }

        if (restartRequested.exchange(false)) {
            LOGI("Restarting AAudio stream after device change/error");
            closeStream();
            if (isPlaying.load()) {
                if (openStream()) {
                    startStream();
                } else {
                    isPlaying.store(false);
                }
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
}

aaudio_data_callback_result_t NativeEngine::dataCallback(
        AAudioStream *stream,
        void *userData,
        void *audioData,
        int32_t numFrames) {
    auto *engine = static_cast<NativeEngine *>(userData);
    if (!engine->isPlaying.load() || engine->restartRequested.load()) {
        return AAUDIO_CALLBACK_RESULT_STOP;
    }

    float *output = static_cast<float *>(audioData);
    const float kAmplitude = 0.2f;
    double phase = engine->phase;
    double increment = engine->phaseIncrement.load();
    const double kTwoPi = 6.2831853071795864769;

    for (int32_t i = 0; i < numFrames; ++i) {
        output[i] = static_cast<float>(std::sin(phase) * kAmplitude);
        phase += increment;
        if (phase >= kTwoPi) {
            phase -= kTwoPi;
        }
    }

    engine->phase = phase;
    return AAUDIO_CALLBACK_RESULT_CONTINUE;
}

void NativeEngine::errorCallback(
        AAudioStream *stream,
        void *userData,
        aaudio_result_t error) {
    auto *engine = static_cast<NativeEngine *>(userData);
    if (!engine->isPlaying.load()) {
        return;
    }

    LOGE("AAudio error callback: %d", error);
    engine->restartRequested.store(true);
}

static NativeEngine *g_engine = nullptr;

extern "C" JNIEXPORT void JNICALL
Java_com_zxx_aaudio_MainActivity_nativeStart(JNIEnv *, jobject) {
    if (!g_engine) {
        g_engine = new NativeEngine();
    }
    g_engine->start();
}

extern "C" JNIEXPORT void JNICALL
Java_com_zxx_aaudio_MainActivity_nativeStop(JNIEnv *, jobject) {
    if (g_engine) {
        g_engine->stop();
    }
}

extern "C" JNIEXPORT void JNICALL
Java_com_zxx_aaudio_MainActivity_nativeRelease(JNIEnv *, jobject) {
    if (g_engine) {
        g_engine->release();
        delete g_engine;
        g_engine = nullptr;
    }
}
