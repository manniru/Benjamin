#ifndef AB_SCENE_H
#define AB_SCENE_H

#include <QObject>
#include <QQuickItem>
#include <QQuickWindow>
#include <shellapi.h>
#include "ab_editor.h"
#include "ab_audio.h"
#include "ab_verify.h"
#include "ab_ler_stat.h"

class AbScene : public QObject
{
    Q_OBJECT
public:
    explicit AbScene(QObject *ui, QObject *parent = nullptr);

    AbEditor  *editor;

private slots:
    void startPauseV();
    void breakTimeout();
    void processKey(int key);
    void setStatus(int status);
    void setStatusAudio(int status);
    void verifierChanged();
    void setCategory();
    void focusWordChanged();
    void createEditor();
    void cacheCreated();
    void deleteAllSamples();

private:
    void fillRecParams();
    void updateAutoCpmplete();
    int getCount(int verifier);

    QObject *root;       // root qml object
    QObject *qml_editor; // editor qml object
    QObject *message; // message qml object
    AbAudio *audio;
    AbVerify *verify;
    AbLerStat *ler_stat;
    AbTelegram *telegram;
    QTimer *break_timer;
};

#endif // AB_SCENE_H
