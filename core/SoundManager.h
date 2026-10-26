#pragma once

#include <QByteArray>
#include <QVector>
#include <QtGlobal>

#if defined(_WIN32) && !defined(__EMSCRIPTEN__)
#include <windows.h>
#include <mmsystem.h>
#endif

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

class SoundManager {
public:
    static SoundManager &instance();

    void init();
    void playHeartbeat(double distanceFactor);
    void playScreech();
    void playFootstep(bool running);
    void playCaught();
    void playEscaped();
    void playCheckpoint();
    void playFlicker();

private:
    SoundManager();
    ~SoundManager() = default;

    void playRawWav(const QByteArray &data);
    static QByteArray generateWav(const QVector<double> &freqs,
                                  const QVector<double> &durations,
                                  double volume = 0.5,
                                  int waveType = 0); // 0=sin, 1=square, 2=noise

    QByteArray m_wavHeartbeat;
    QByteArray m_wavFastHeartbeat;
    QByteArray m_wavScreech;
    QByteArray m_wavStepWalk;
    QByteArray m_wavStepRun;
    QByteArray m_wavCaught;
    QByteArray m_wavEscaped;
    QByteArray m_wavCheckpoint;
    QByteArray m_wavFlicker;

    qint64 m_lastHeartbeatTime = 0;
    qint64 m_lastStepTime = 0;
};
