#ifndef ABSCENE_H
#define ABSCENE_H

#include <QObject>
#include <QQuickItem>
#include <QQuickWindow>
#include "ab_manager.h"
#include "ab_stat.h"

class AbScene : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(QString category READ category WRITE setCategory NOTIFY categoryChanged)
    Q_PROPERTY(QString words READ words WRITE setWords NOTIFY wordsChanged)
    Q_PROPERTY(QString stat READ stat WRITE setStat NOTIFY statChanged)
    Q_PROPERTY(qreal count READ count WRITE setCount NOTIFY countChanged)
    Q_PROPERTY(qreal totalcount READ totalcount WRITE setTotalcount NOTIFY totalcountChanged)
    Q_PROPERTY(qreal elapsedtime READ elapsedtime WRITE setElapsedtime NOTIFY elapsedtimeChanged)
    Q_PROPERTY(qreal status READ status WRITE setStatus NOTIFY statusChanged)
    Q_PROPERTY(qreal rectime READ rectime WRITE setRectime NOTIFY rectimeChanged)
    Q_PROPERTY(qreal numwords READ numwords WRITE setNumwords NOTIFY numwordsChanged)
    Q_PROPERTY(qreal pausetime READ pausetime WRITE setPausetime NOTIFY pausetimeChanged)
    Q_PROPERTY(qreal key READ key WRITE setKey NOTIFY keyChanged)

    Q_PROPERTY(qreal qmlcreated READ qmlcreated WRITE setQmlcreated NOTIFY qmlcreatedChanged)

public:
    AbScene();

    QString category() const { return man->params.category; }
    void setCategory(QString category);
    QString words() const { return man->params.words; }
    QString stat() const { return man->params.stat; }
    void setStat(QString stat);
    qreal count() const { return man->params.count; }
    qreal totalcount() const { return man->params.total_count; }
    void setTotalcount(qreal totalcount);
    qreal elapsedtime() const { return man->params.elapsed_time; }
    qreal status() const { return man->params.status; }
    void setStatus(qreal status);
    qreal rectime() const { return man->params.rec_time; }
    void setRectime(qreal rectime);
    qreal numwords() const { return man->params.num_words; }
    void setNumwords(qreal numwords);
    qreal pausetime() const { return man->params.pause_time; }
    void setPausetime(qreal pausetime);
    qreal key() const { return man->params.key; }
    void setKey(qreal key);

    qreal qmlcreated() const {return m_qmlcreated;}
    void setQmlcreated(qreal qmlcreated);

public slots:
    void updateStatus(qreal status);
    void setWords(QString words);
    void setCount(qreal count);
    void setElapsedtime(qreal elapsedtime);

signals:
    void categoryChanged();
    void wordsChanged();
    void statChanged();
    void countChanged();
    void totalcountChanged();
    void elapsedtimeChanged();
    void statusChanged();
    void rectimeChanged();
    void numwordsChanged();
    void pausetimeChanged();
    void keyChanged();

    void qmlcreatedChanged();

private:
    void fillRecParams();
    void processKey(int key);

    AbManager *man;
    AbRecordParam *rec_params;
    qreal m_qmlcreated=0;
};

void ab_setUi(QObject *ui);
QObject* pna_getUi();

#endif // ABSCENE_H
