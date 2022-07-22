#ifndef CH_CHAPAR_H
#define CH_CHAPAR_H

#include <QObject>
#include <QThread>

#ifdef WIN32
    #include "ch_channel_w.h"
    #include "ch_processor_w.h"
#else
    #include "ch_channel_l.h"
    #include "ch_processor_l.h"
#endif

class ChChapar : public QObject
{
    Q_OBJECT
public:
    ChChapar(QObject *ui, QObject *parent = nullptr);

signals:
    void run();

private:
    QThread *ch_thread;

};

#endif // CH_CHAPAR_H
