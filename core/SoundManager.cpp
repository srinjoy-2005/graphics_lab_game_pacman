#include "core/SoundManager.h"
#include <QDateTime>
#include <cmath>
#include <random>

SoundManager &SoundManager::instance() {
    static SoundManager inst;
    return inst;
}

SoundManager::SoundManager() {
    init();
}

void SoundManager::init() {
    // Generate Heartbeat (low frequency thud)
    m_wavHeartbeat = generateWav({55.0, 45.0, 0.0, 50.0, 40.0}, {0.08, 0.06, 0.05, 0.07, 0.08}, 0.6, 0);
    m_wavFastHeartbeat = generateWav({70.0, 55.0, 0.0, 65.0, 50.0}, {0.05, 0.04, 0.03, 0.05, 0.05}, 0.8, 0);

    // Entity Screech (dissonant horror screech)
    m_wavScreech = generateWav({440.0, 880.0, 1174.0, 1568.0, 2200.0, 800.0},
                               {0.05, 0.05, 0.06, 0.08, 0.12, 0.15}, 0.7, 1);

    // Footsteps
    m_wavStepWalk = generateWav({80.0, 40.0}, {0.03, 0.03}, 0.25, 0);
    m_wavStepRun  = generateWav({110.0, 60.0}, {0.02, 0.03}, 0.40, 0);

    // Caught / Jumpscare (harsh noise + drop)
    m_wavCaught   = generateWav({900.0, 450.0, 180.0, 60.0, 30.0}, {0.10, 0.12, 0.20, 0.30, 0.30}, 0.9, 1);

    // Escaped (ethereal win tone)
    m_wavEscaped  = generateWav({261.6, 329.6, 392.0, 523.2, 659.2}, {0.15, 0.15, 0.15, 0.20, 0.40}, 0.5, 0);

    // Checkpoint chime (comforting chord)
    m_wavCheckpoint = generateWav({523.2, 659.2, 783.99}, {0.10, 0.10, 0.25}, 0.45, 0);

    // Flashlight flicker spark
    m_wavFlicker  = generateWav({1200.0, 200.0, 1800.0}, {0.02, 0.02, 0.03}, 0.35, 1);
}

void SoundManager::playRawWav(const QByteArray &data) {
#if defined(_WIN32) && !defined(__EMSCRIPTEN__)
    if (!data.isEmpty()) {
        PlaySoundA(reinterpret_cast<LPCSTR>(data.constData()), NULL, SND_MEMORY | SND_ASYNC);
    }
#elif defined(__EMSCRIPTEN__)
    if (!data.isEmpty()) {
        // Base64 Web Audio playback in browser environment
        QString b64 = QString::fromLatin1(data.toBase64());
        QString js = QString("var a = new Audio('data:audio/wav;base64,%1'); a.volume = 0.6; a.play().catch(function(){});").arg(b64);
        emscripten_run_script(js.toUtf8().constData());
    }
#endif
}

void SoundManager::playHeartbeat(double distanceFactor) {
    qint64 now = QDateTime::currentMSecsSinceEpoch();
    // distanceFactor from 0.0 (right on top of player) to 1.0 (far away)
    int interval = static_cast<int>(350 + distanceFactor * 1200);
    if (now - m_lastHeartbeatTime >= interval) {
        m_lastHeartbeatTime = now;
        if (distanceFactor < 0.35) {
            playRawWav(m_wavFastHeartbeat);
        } else {
            playRawWav(m_wavHeartbeat);
        }
    }
}

void SoundManager::playScreech() {
    playRawWav(m_wavScreech);
}

void SoundManager::playFootstep(bool /*running*/) {
    // Footstep movement sound turned off as requested
}

void SoundManager::playCaught() {
    playRawWav(m_wavCaught);
}

void SoundManager::playEscaped() {
    playRawWav(m_wavEscaped);
}

void SoundManager::playCheckpoint() {
    playRawWav(m_wavCheckpoint);
}

void SoundManager::playFlicker() {
    playRawWav(m_wavFlicker);
}

QByteArray SoundManager::generateWav(const QVector<double> &freqs,
                                     const QVector<double> &durations,
                                     double volume,
                                     int waveType) {
    const uint32_t sampleRate = 22050;
    int totalSamples = 0;
    for (double d : durations) {
        totalSamples += static_cast<int>(sampleRate * d);
    }

    uint32_t dataSize = totalSamples;
    uint32_t chunkSize = 36 + dataSize;
    QByteArray buf;
    buf.reserve(44 + dataSize);

    buf.append("RIFF", 4);
    buf.append(reinterpret_cast<const char*>(&chunkSize), 4);
    buf.append("WAVEfmt ", 8);
    uint32_t sub1 = 16;
    uint16_t fmt = 1, channels = 1;
    uint32_t sr = sampleRate, br = sampleRate;
    uint16_t align = 1, bits = 8;
    buf.append(reinterpret_cast<const char*>(&sub1), 4);
    buf.append(reinterpret_cast<const char*>(&fmt), 2);
    buf.append(reinterpret_cast<const char*>(&channels), 2);
    buf.append(reinterpret_cast<const char*>(&sr), 4);
    buf.append(reinterpret_cast<const char*>(&br), 4);
    buf.append(reinterpret_cast<const char*>(&align), 2);
    buf.append(reinterpret_cast<const char*>(&bits), 2);
    buf.append("data", 4);
    buf.append(reinterpret_cast<const char*>(&dataSize), 4);

    const double PI = 3.14159265358979323846;
    for (int note = 0; note < freqs.size(); ++note) {
        double freq = freqs[note];
        double dur = durations[note];
        int nSamples = static_cast<int>(sampleRate * dur);
        for (int i = 0; i < nSamples; ++i) {
            uint8_t sample = 128;
            if (freq > 0.0) {
                double t = static_cast<double>(i) / sampleRate;
                double phase = std::fmod(t * freq, 1.0);
                double val = 0.0;
                if (waveType == 1) {
                    val = (phase < 0.5) ? 1.0 : -1.0;
                } else {
                    val = std::sin(2.0 * PI * phase);
                }
                double env = 1.0;
                if (i > nSamples - 80) env = static_cast<double>(nSamples - i) / 80.0;
                sample = static_cast<uint8_t>(qBound(0, static_cast<int>(128 + val * 127.0 * volume * env), 255));
            }
            buf.append(static_cast<char>(sample));
        }
    }
    return buf;
}
