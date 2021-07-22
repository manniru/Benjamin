#ifndef RESTATE_H
#define RESTATE_H

#include <QObject>
#include <QString>

typedef struct ReWindow
{
    int  type;
    QString title;
    QString pname;
}ReWindow;

class ReState : public QObject
{
    Q_OBJECT
public:
    explicit ReState(QObject *parent = 0);
    void setMode(int mode);
    int getMode();

    ReWindow app; //Active Window

signals:
    void updateMode();

private:
    int i_mode;
};

#endif // RESTATE_H
