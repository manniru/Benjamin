#include "ab_scene.h"

QObject* root;//root qml object

AbScene::AbScene()
{
    man = new AbManager();
    connect(man, SIGNAL(wordsChanged(QString)),
            this, SLOT(setWords(QString)));
    connect(man, SIGNAL(statusChanged(qreal)),
            this, SLOT(updateStatus(qreal)));
    connect(man, SIGNAL(countChanged(qreal)),
            this, SLOT(setCount(qreal)));
    connect(man, SIGNAL(timeChanged(qreal)),
            this, SLOT(setElapsedtime(qreal)));
}

void AbScene::setQmlcreated(qreal qmlcreated)
{
    if( qmlcreated==m_qmlcreated )
    {
        return;
    }
    m_qmlcreated = qmlcreated;
    emit qmlcreatedChanged();
    if(window())
    {
        window()->update();
    }
}

void AbScene::setTotalcount(qreal totalcount)
{
    if( totalcount==man->params.total_count )
    {
        return;
    }
    man->params.total_count = totalcount;
    emit totalcountChanged();
    if( window() )
    {
        window()->update();
    }
}

void AbScene::setElapsedtime(qreal elapsedtime)
{
    if( elapsedtime==man->params.elapsed_time )
    {
        return;
    }
    man->params.elapsed_time = elapsedtime;
    emit elapsedtimeChanged();
    if( window() )
    {
        window()->update();
    }
}

void AbScene::setStatus(qreal status)
{
    if( status==man->params.status )
    {
        return;
    }
    man->params.status = status;

    if( man->params.status==AB_STATUS_REC )
    {
        man->record();
    }
    else if( man->params.status==AB_STATUS_STOP )
    {

    }
    else if( man->params.status==AB_STATUS_PAUSE )
    {

    }

    emit statusChanged();
    if( window() )
    {
        window()->update();
    }
}

void AbScene::updateStatus(qreal status)
{
    if( status==man->params.status )
    {
        return;
    }
    man->params.status = status;

    emit statusChanged();
    if( window() )
    {
        window()->update();
    }
}

void AbScene::setRectime(qreal rectime)
{
    if( rectime==man->params.rec_time )
    {
        return;
    }
    man->params.rec_time = man->params.rec_time;
    emit rectimeChanged();
    if( window() )
    {
        window()->update();
    }
}

void AbScene::setNumwords(qreal numwords)
{
    if( numwords==man->params.num_words )
    {
        return;
    }
    man->params.num_words = numwords;
    emit numwordsChanged();
    if( window() )
    {
        window()->update();
    }
}

void AbScene::setPausetime(qreal pausetime)
{
    if( pausetime==man->params.pause_time )
    {
        return;
    }
    man->params.pause_time = pausetime;
    emit pausetimeChanged();
    if( window() )
    {
        window()->update();
    }
}

void AbScene::setCount(qreal count)
{
    if( count==man->params.count )
    {
        return;
    }
    man->params.count = count;
    emit countChanged();
    if( window() )
    {
        window()->update();
    }
}

void AbScene::setCategory(QString category)
{
    if( category==man->params.category )
    {
        return;
    }

    man->params.category = category;
    emit categoryChanged();
    if( window() )
    {
        window()->update();
    }
}

void AbScene::setWords(QString words)
{
    if( words==man->params.words )
    {
        return;
    }

    man->params.words = words;
    emit wordsChanged();
    if( window() )
    {
        window()->update();
    }
}

void ab_setUi(QObject *ui)
{
    root = ui;
}

QObject* pna_getUi()
{
    return root;
}
