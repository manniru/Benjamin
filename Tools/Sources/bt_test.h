#ifndef BT_RECORDER_H
#define BT_RECORDER_H

// BT Test Mode
// Use wave file instead of online
// recorder

#include <QObject>
#include <QTimer>
#include <QDebug>
#include <QThread>

class BtTest: public QObject
{
    Q_OBJECT
public:
    explicit BtTest(QObject *parent = nullptr);
    ~BtTest();

private:

};

#endif // BT_RECORDER_H
