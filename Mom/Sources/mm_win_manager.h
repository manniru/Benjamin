#ifndef RE_WIN_MANAGER_H
#define RE_WIN_MANAGER_H

#include <QVector>
#include <QObject>
#include <QTimer>
#include <windows.h>
#include "mm_config.h"

class MmWinManager: public QObject
{
    Q_OBJECT
public:
    explicit MmWinManager(QObject *parent = nullptr);
    ~MmWinManager();

    void restore();
    void maximise();
    void minimise();
    void putLeft();
    void putRight();

private:
    QVector<int> x_left;
    QVector<int> width;
};

#endif // RE_WIN_MANAGER_H
