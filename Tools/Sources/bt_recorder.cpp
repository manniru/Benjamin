#include "bt_recorder.h"
#include <QDebug>

BtRecoder::BtRecoder(QObject *parent) : QObject(parent)
{
    recorder = new QAudioRecorder(this);

    QAudioEncoderSettings audioSettings;
    audioSettings.setCodec("audio/pcm");
//    audioSettings.setQuality(QMultimedia::HighQuality);
    audioSettings.setChannelCount(1);
    audioSettings.setSampleRate(BT_REC_RATE);

    recorder->setAudioInput(BT_REC_INPUT);
    recorder->setEncodingSettings(audioSettings);

    record_timer = new QTimer;

    connect(record_timer, SIGNAL(timeout()), this, SLOT(recordTimeout()));



//    qDebug() << recorder->audioInput();
//    QStringList codecs_list = recorder->audioInputs();

//    for( int i=0 ; i<codecs_list.count() ; i++ )
//    {
//        qDebug() << codecs_list[i];
//    }
}

void BtRecoder::start()
{
    recorder->setOutputLocation(QUrl::fromLocalFile("test.wav"));
    recorder->record();

    record_timer->start(5000);
    emit resultReady();
}



void BtRecoder::recordTimeout()
{
    record_timer->stop();
    recorder->stop();
}
