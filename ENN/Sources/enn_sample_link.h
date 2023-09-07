#ifndef ENNSAMPLELINK_H
#define ENNSAMPLELINK_H

#include <QObject>
#include <QQuickImageProvider>
#include "backend.h"

#define BT_ENN_SIZE 40

// UI Sample Image Loader
class EnnSampleLink : public QObject, public QQuickImageProvider
{
    Q_OBJECT
public:
    explicit EnnSampleLink(QObject *parent = nullptr);

    QPixmap requestPixmap(const QString &id, QSize *size,
                          const QSize &requestedSize) override;

signals:

private:
    void calcStat(tiny_dnn::vec_t data);

    QString enn_path;
    double offset_delta = -5;
    double scale_delta = 19;
};

#endif // ENNSAMPLELINK_H
