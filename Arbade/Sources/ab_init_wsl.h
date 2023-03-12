#ifndef AB_INIT_WSL_H
#define AB_INIT_WSL_H

#include <QObject>
#include <QDir>

class AbInitWSL : public QObject
{
    Q_OBJECT
public:
    explicit AbInitWSL(QObject *parent = nullptr);
    ~AbInitWSL();

    QString getWslPath();
};

#endif // AB_INIT_WSL_H
