#ifndef MM_BAR_H
#define MM_BAR_H

#include <QObject>
#include <QWindow>
#include "mm_config.h"
#include "mm_music.h"
#include "mm_parser.h"
#include "mm_sound.h"
#include "mm_usage.h"
#include "mm_virt.h"
#include "mm_channel.h"

class MmBar : public QObject
{
    Q_OBJECT
public:
    explicit MmBar(QVector<QWindow *> windows, MmVirt *vi, MmSound *snd,
                   QObject *parent = nullptr);

signals:
    void startChannel();

private slots:
    void executeAction(QString action);
    void loadLabels();

private:
    void    parseProps(QString raw, MmProperty *properties);
    int updateWorkSpace(); // add workspace ID
    int addRWidget();  // add sound widget
    QString getWorkStr(int index);
    void    addLabel(QObject *bar, MmLabel *label);
    void    updateUILabel(QObject *bar, int id, MmLabel *label);
    int     proccessFile(QVector<QObject *> bars, int s_id, QString path);
    void    updateLabel(QObject *bar, int index, MmLabel *new_lbl);
    int     isChanged(MmLabel *label1, MmLabel *label2);

    QVector<QObject *> left_bars;
    QVector<QObject *> right_bars;

    int l_id; //left  id
    int r_id; //right id

    QTimer    *timer;
    MmParser  *parser;
    MmVirt    *virt;
    MmSound   *sound;
    MmUsage   *usage;
    MmMusic   *music;
    MmChannel *channel;
    QThread   *channel_thread;
    QVector<QWindow *> wins;

    // These are buffers to help reduce number of updates
    // requests need
    QVector<MmLabel> r_labels; //left labels
    QVector<MmLabel> l_labels; //right label
};

#endif // MM_BAR_H
