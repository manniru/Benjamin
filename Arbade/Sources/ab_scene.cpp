#include "ab_scene.h"

QObject* root;//root qml object

AbScene::AbScene()
{
    man = new AbManager();
    break_timer = new QTimer();
    connect(man, SIGNAL(wordsChanged(QString)),
            this, SLOT(setWords(QString)));
    connect(man, SIGNAL(statusChanged(qreal)),
            this, SLOT(updateStatus(qreal)));
    connect(man, SIGNAL(countChanged(qreal)),
            this, SLOT(setCount(qreal)));
    connect(man, SIGNAL(timeChanged(qreal)),
            this, SLOT(setElapsedtime(qreal)));
    connect(man, SIGNAL(powerChanged(qreal)),
            this, SLOT(setPower(qreal)));

    connect(man, SIGNAL(pauseChanged(qreal)),
            this, SLOT(setPausetime(qreal)));
    connect(man, SIGNAL(numWordChanged(qreal)),
            this, SLOT(setNumwords(qreal)));
    connect(man, SIGNAL(recTimeChanged(qreal)),
            this, SLOT(setRectime(qreal)));
    connect(man, SIGNAL(totalCountChanged(qreal)),
            this, SLOT(setTotalcount(qreal)));
    connect(man, SIGNAL(categoryChanged(QString)),
            this, SLOT(setCategory(QString)));

    connect(break_timer, SIGNAL(timeout()),
            this, SLOT(breakTimeout()));
}

void AbScene::setQmlcreated(qreal qmlcreated)
{
    if( qmlcreated==m_qmlcreated )
    {
        return;
    }
    m_qmlcreated = qmlcreated;
    setStat(ab_getStat(man->params.category));

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

    if( status==AB_STATUS_REC && man->params.verifier==0 )
    {
        man->record();
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

    if( status==AB_STATUS_STOP )
    {
        setStat(ab_getStat(man->params.category));
    }

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
    man->params.rec_time = rectime;
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

void AbScene::setKey(qreal key)
{
    if( key==man->params.key )
    {
        return;
    }
    processKey(key);
    man->params.key = 0;
    emit keyChanged();
    if( window() )
    {
        window()->update();
    }
}

void AbScene::setPower(qreal power)
{
    if( power==man->params.power )
    {
        return;
    }
    man->params.power = power;
    emit powerChanged();
    if( window() )
    {
        window()->update();
    }
}

void AbScene::setVerifier(qreal verifier)
{
    if( verifier==man->params.verifier )
    {
        return;
    }
    man->params.verifier = verifier;
    man->swapParams();
    setStat(ab_getStat(man->params.category));
    setCount(0);

    if( verifier )
    {
        unverified_list = ab_listFiles(KAL_AU_DIR_WIN"unverified\\",
                                       AB_LIST_PATHS);
        setTotalcount(unverified_list.size());
    }

    emit verifierChanged();
    if( window() )
    {
        window()->update();
    }
}

void AbScene::setLoadsrc(qreal loadsrc)
{
    if( loadsrc==man->params.loadsrc )
    {
        return;
    }
    man->params.loadsrc = 0;
    if( man->params.verifier==1 )
    {
        if( man->params.status!=AB_STATUS_STOP ) // not for 1st time
        {
            if( man->params.delfile ) // deleted shit file
            {
                setDelfile(0);
            }
            else // verified
            {
                man->copyToOnline(unverified_list[man->params.count-1]);
            }
        }
        if( loadsrc && man->params.count<man->params.total_count )
        {
            setStatus(AB_STATUS_BREAK);
            setAddress(unverified_list[man->params.count]);
            setCount(man->params.count+1);
            break_timer->start(man->params.pause_time*1000);
        }
        else if( loadsrc ) // cnt>=total
        {
            setStatus(AB_STATUS_STOP);
        }
    }
    emit loadsrcChanged();
    if( window() )
    {
        window()->update();
    }
}

void AbScene::setDelfile(qreal delfile)
{
    if( delfile==man->params.delfile )
    {
        return;
    }
    if( delfile==1 )
    {
        QFile file(unverified_list[man->params.count-1]);
        file.remove();
    }
    man->params.delfile = delfile;
    emit delfileChanged();
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
    setStat(ab_getStat(man->params.category));
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

void AbScene::setStat(QString stat)
{
    if( stat==man->params.stat )
    {
        return;
    }

    man->params.stat = stat;
    updateCategories();
    emit statChanged();
    if( window() )
    {
        window()->update();
    }
}

void AbScene::setFocusword(QString focusword)
{
    if( focusword==man->params.focusword )
    {
        return;
    }

    man->params.focusword = focusword;
    emit focuswordChanged();
    if( window() )
    {
        window()->update();
    }
}

void AbScene::setWordlist(QString wordlist)
{
    if( wordlist==man->params.wordlist )
    {
        return;
    }

    if( wordlist=="request data" ) // load phase
    {
        man->params.wordlist = man->readWordList();
        setWordstat(ab_getStat());
    }
    else // save phase
    {
        man->params.wordlist = wordlist;
        man->writeWordList();
        man->params.wordlist = "";
    }

    emit wordlistChanged();
    if( window() )
    {
        window()->update();
    }
}

void AbScene::setWordstat(QString wordstat)
{
    if( wordstat==man->params.wordstat )
    {
        return;
    }

    man->params.wordstat = wordstat;
    emit wordstatChanged();
    if( window() )
    {
        window()->update();
    }
}

void AbScene::setDifwords(QString difwords)
{
    if( difwords==man->params.difwords )
    {
        return;
    }

    man->params.difwords = difwords;
    man->delWordSamples();
    man->params.difwords = "";
    emit difwordsChanged();
    if( window() )
    {
        window()->update();
    }
}

void AbScene::setAutocomp(QString autocomp)
{
    if( autocomp==man->params.autocomp )
    {
        return;
    }

    man->params.autocomp = autocomp;
    emit autocompChanged();
    if( window() )
    {
        window()->update();
    }
}

void AbScene::setMeanvar(QString meanvar)
{
    if( meanvar==man->params.meanvar )
    {
        return;
    }

    man->params.meanvar = meanvar;
    emit meanvarChanged();
    if( window() )
    {
        window()->update();
    }
}

void AbScene::setPlaykon(qreal playkon)
{
    if( playkon==man->params.playkon )
    {
        return;
    }

    man->params.playkon = playkon;
    emit playkonChanged();
    if( window() )
    {
        window()->update();
    }
}

void AbScene::setAddress(QString address)
{
    if( address==man->params.address )
    {
        return;
    }

    man->readWave(address);
    man->params.address = address;
    emit addressChanged();
    if( window() )
    {
        window()->update();
    }
}

void AbScene::breakTimeout()
{
    setStatus(AB_STATUS_PLAY);
    if( man->params.playkon )
    {
        setPlaykon(0);
    }
    else
    {
        setPlaykon(1);
    }
    break_timer->stop();
}

void AbScene::processKey(int key)
{
    if( key==Qt::Key_T )
    {
        QString path = QDir::currentPath() + "\\";
        path.replace('/', '\\');
        path +=  KAL_AU_DIR_WIN"train\\";
        path += man->params.category + "\\";
        QString cmd = "explorer.exe " + path;
        system(cmd.toStdString().c_str());
    }
    else
    {
        return;
    }
}

void AbScene::updateCategories()
{
    QFileInfoList dir_list = ab_getAudioDirs();
    QStringList categories;
    int len = dir_list.size();
    for( int i=0 ; i<len ; i++)
    {
        categories.append(dir_list[i].fileName());
    }
    QString cat_str = categories.join("!");
    setAutocomp(cat_str);
}

void ab_setUi(QObject *ui)
{
    root = ui;
}

QObject* ab_getUi()
{
    return root;
}
