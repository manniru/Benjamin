#ifndef MM_BAR_H
#define MM_BAR_H

#include <QObject>
#include "mm_config.h"
#include "mm_parser.h"
#include "mm_virt.h"

class MmBar : public QObject
{
    Q_OBJECT
public:
    explicit MmBar(QObject *root, MmVirt *vi,
                   QObject *parent = nullptr);

private slots:
    void executeCommand(QString action);
    void loadLabels();

private:
    void    parseProps(QString raw, MmProperty *properties);
    void    addWorkID(); // add workspace ID
    QString getWorkStr(int index);
    void    addLabel(MmLabel label);
    void    updateLabel(int id, MmLabel label);
    int     proccessFile(int s_id, QString path);
    void    updateLbl(int id, MmLabel new_lbl);

    QObject *left_bar;
    QObject *right_bar;
    QObject *side;

    int l_id; //left  id
    int r_id; //right id

    QTimer   *timer;
    MmParser *parser;
    MmVirt   *virt;

    // These are buffers to help reduce number of updates
    // requests need
    QVector<MmLabel> r_labels; //left labels
    QVector<MmLabel> l_labels; //right label
};

#endif // MM_BAR_H
