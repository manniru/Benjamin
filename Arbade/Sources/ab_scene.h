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
    Q_PROPERTY(QString address READ address WRITE setAddress NOTIFY addressChanged)
    Q_PROPERTY(QString focusword READ focusword WRITE setFocusword NOTIFY focuswordChanged)
    Q_PROPERTY(QString wordlist READ wordlist WRITE setWordlist NOTIFY wordlistChanged)
    Q_PROPERTY(QString wordstat READ wordstat WRITE setWordstat NOTIFY wordstatChanged)
    Q_PROPERTY(QString difwords READ difwords WRITE setDifwords NOTIFY difwordsChanged)
    Q_PROPERTY(qreal count READ count WRITE setCount NOTIFY countChanged)
    Q_PROPERTY(qreal totalcount READ totalcount WRITE setTotalcount NOTIFY totalcountChanged)
    Q_PROPERTY(qreal elapsedtime READ elapsedtime WRITE setElapsedtime NOTIFY elapsedtimeChanged)
    Q_PROPERTY(qreal status READ status WRITE setStatus NOTIFY statusChanged)
    Q_PROPERTY(qreal rectime READ rectime WRITE setRectime NOTIFY rectimeChanged)
    Q_PROPERTY(qreal numwords READ numwords WRITE setNumwords NOTIFY numwordsChanged)
    Q_PROPERTY(qreal pausetime READ pausetime WRITE setPausetime NOTIFY pausetimeChanged)
    Q_PROPERTY(qreal key READ key WRITE setKey NOTIFY keyChanged)
    Q_PROPERTY(qreal power READ power WRITE setPower NOTIFY powerChanged)
    Q_PROPERTY(qreal verifier READ verifier WRITE setVerifier NOTIFY verifierChanged)
    Q_PROPERTY(qreal loadsrc READ loadsrc WRITE setLoadsrc NOTIFY loadsrcChanged)
    Q_PROPERTY(qreal delfile READ delfile WRITE setDelfile NOTIFY delfileChanged)
    Q_PROPERTY(qreal playkon READ playkon WRITE setPlaykon NOTIFY playkonChanged)

    Q_PROPERTY(qreal qmlcreated READ qmlcreated WRITE setQmlcreated NOTIFY qmlcreatedChanged)

public:
    AbScene();

    QString category() const { return man->params.category; }
    QString words() const { return man->params.words; }
    QString stat() const { return man->params.stat; }
    void setStat(QString stat);
    QString address() const { return man->params.address; }
    QString focusword() const { return man->params.focusword; }
    void setFocusword(QString focusword);
    QString wordlist() const { return man->params.wordlist; }
    void setWordlist(QString wordlist);
    QString wordstat() const { return man->params.wordstat; }
    void setWordstat(QString wordstat);
    QString difwords() const { return man->params.difwords; }
    void setDifwords(QString difwords);
    qreal count() const { return man->params.count; }
    qreal totalcount() const { return man->params.total_count; }
    qreal elapsedtime() const { return man->params.elapsed_time; }
    qreal status() const { return man->params.status; }
    void setStatus(qreal status);
    qreal rectime() const { return man->params.rec_time; }
    qreal numwords() const { return man->params.num_words; }
    qreal pausetime() const { return man->params.pause_time; }
    qreal key() const { return man->params.key; }
    void setKey(qreal key);
    qreal power() const { return man->params.power; }
    qreal verifier() const { return man->params.verifier; }
    void setVerifier(qreal verifier);
    qreal loadsrc() const { return man->params.loadsrc; }
    void setLoadsrc(qreal loadsrc);
    qreal delfile() const { return man->params.delfile; }
    void setDelfile(qreal delfile);
    qreal playkon() const { return man->params.playkon; }
    void setPlaykon(qreal playkon);

    qreal qmlcreated() const {return m_qmlcreated;}
    void setQmlcreated(qreal qmlcreated);

public slots:
    void updateStatus(qreal status);
    void setWords(QString words);
    void setCount(qreal count);
    void setElapsedtime(qreal elapsedtime);
    void setPower(qreal power);
    void setPausetime(qreal pausetime);
    void setNumwords(qreal numwords);
    void setRectime(qreal rectime);
    void setTotalcount(qreal totalcount);
    void setCategory(QString category);
    void setAddress(QString address);
    void breakTimeout();

signals:
    void categoryChanged();
    void wordsChanged();
    void statChanged();
    void addressChanged();
    void focuswordChanged();
    void wordlistChanged();
    void wordstatChanged();
    void difwordsChanged();
    void countChanged();
    void totalcountChanged();
    void elapsedtimeChanged();
    void statusChanged();
    void rectimeChanged();
    void numwordsChanged();
    void pausetimeChanged();
    void keyChanged();
    void powerChanged();
    void verifierChanged();
    void loadsrcChanged();
    void delfileChanged();
    void playkonChanged();

    void qmlcreatedChanged();

private:
    void fillRecParams();
    void processKey(int key);

    AbManager *man;
    AbRecParam *rec_params;
    QTimer *break_timer;
    QStringList unverified_list;
    qreal m_qmlcreated=0;
};

void ab_setUi(QObject *ui);
QObject* pna_getUi();

#endif // ABSCENE_H
