#ifndef BT_STATE_H
#define BT_STATE_H

#include <QObject>
#include <QString>

typedef struct ReWindow
{
    int  type;
    QString title;
    QString pname;
}ReWindow;

class BtState : public QObject
{
    Q_OBJECT
public:
    explicit BtState(QObject *parent = 0);
    void setMode(int mode);
    int getMode();

    ReWindow app; //Active Window

signals:
    void updateMode();

private:
    int i_mode;
};

#endif // BT_STATE_H
