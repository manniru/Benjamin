#include "bt_recorder.h"
#include <QDebug>

BtRecoder::BtRecoder(QThread *thread, QObject *parent) : QObject(parent)
{


    record_timer = new QTimer;
    record_timer->moveToThread(thread);

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
    recorder = new QAudioRecorder(this);

    QAudioEncoderSettings audioSettings;
    audioSettings.setCodec("audio/mpeg, mpegversion=(int)4");
//    audioSettings.setQuality(QMultimedia::HighQuality);
    audioSettings.setChannelCount(2);
    audioSettings.setSampleRate(BT_REC_RATE);

//    recorder->setAudioInput(BT_REC_INPUT);
    recorder->setEncodingSettings(audioSettings);

    recorder->setOutputLocation(QUrl::fromLocalFile("test.mp4"));
    recorder->record();

    record_timer->start(5000);
    emit resultReady();
}



void BtRecoder::recordTimeout()
{
    record_timer->stop();
    recorder->stop();
}
