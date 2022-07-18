#include "bt_captain.h"
#include <QDebug>
#include <QColor>

BtCaptain::BtCaptain(BtState *state,
                     QObject *parent) : QObject(parent)
{
    setbuf(stdout,NULL); //to work out printf

    time_shifter = new QTimer;
    connect(time_shifter, SIGNAL(timeout()),
            this, SLOT(shiftHistory()));
    time_shifter->start(BT_HISTORY_UPDATE);

    start_treshold = -BT_HISTORY_SIZE/1000.0;
    net = new BtNetwork;
    st  = state;

    /// FIXME: ADD STRICT TO THE INI FILE
//    strict_word << "five";
//    strict_word << "four";
    strict_word << "go";
#ifdef WIN32
    lua = new BtLua();
#endif
}

BtCaptain::~BtCaptain()
{
#ifdef WIN32
    delete lua;
#endif
}

void BtCaptain::parse(QVector<BtWord> in_words, uint max_frame)
{
    if( in_words.empty() )
    {
        return;
    }

    x_buf = ""; //fill inside add word
    for( int i=0 ; i<in_words.length() ; i++ )
    {
        addWord(in_words[i], i);
    }
    exec(x_buf);
    syncFrame(max_frame);
    writeResult();
}

void BtCaptain::syncFrame(uint max_frame)
{
    double theoretical = max_frame/100.0;
    theoretical -= BT_HISTORY_SIZE/1000.0;

    float diff = qAbs(start_treshold-theoretical);
    if( diff>BT_MAXSYNC_DIFF )
    {
        start_treshold = theoretical;
    }
}

void BtCaptain::exec(QString word)
{
    if( x_buf.isEmpty() )
    {
        return;
    }

    QString cmd;
#ifdef WIN32
    lua->run(word);
#else
    cmd = KAL_SI_DIR"main_l.sh \"";
    cmd += word;
    cmd += "\"";
    system(cmd.toStdString().c_str());
#endif
    qDebug() << "exec" << x_buf;
}

void BtCaptain::addWord(BtWord word, int id)
{
    BtHistory buf;
    // change not final last to final
    // remove all that are not final
    if( id<current.length() )
    {
        if( current[id].words.last()!=word.word )
        {
            if( current[id].is_final )
            {
                current[id].words.push_back(word.word);
            }
            else
            {
                int last_id = current[id].words.length()-1;
                current[id].words[last_id] = word.word;
            }
            addXBuf(word);
        }
        else if( current[id].is_final==0 ) //and same word
        {
            addXBuf(word);
        }
        if( word.is_final )
        {
            current[id].conf = getConf(word);
            current[id].time = word.time;
            current[id].is_final = 1;
        }
        return;
    }

    addXBuf(word);
    buf.words.append(word.word);
    buf.conf = getConf(word);
    buf.time = word.time;
    buf.is_final = word.is_final;

    current.append(buf);
}

void BtCaptain::addXBuf(BtWord word)
{
    if( strict_word.contains(word.word) )
    {
        word.conf = getConf(word);
        qDebug() << "Strict Word" << word.word
                 << st->hard_threshold
                 << word.conf;
        if( word.conf>0.3 )
        {
            word.conf = 1.0;
        }
    }
    if( word.conf<st->hard_threshold )
    {
        return;
    }
    if( word.is_final )
    {
        if( x_buf.length() )
        {
            x_buf += " ";
        }
        x_buf += word.word;
    }
}

void BtCaptain::shiftHistory()
{
    start_treshold += BT_HISTORY_UPDATE/1000.0;

    for( int i=0 ; i<history.length() ; i++ )
    {
        if( (history[i].time<start_treshold) || //5 second history size
            (history.length()>BT_HISTORY_LEN) ) // max len = 8
        {
            history.remove(i);
            i--;
        }
    }

    writeResult();
}

void BtCaptain::writeResult()
{
    QFile bar_file(BT_BAR_RESULT);

    if( !bar_file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qDebug() << "Error creating" << BT_BAR_RESULT;
        qDebug() << "Try create Resource Folder";
        system("mkdir Resources");
        return;
    }
    QTextStream out(&bar_file);

    for( int i=0 ; i<history.length() ; i++ )
    {
        out << getWordFmt(history[i]);
    }

    for( int i=0 ; i<current.length() ; i++ )
    {
        if( current[i].time>start_treshold ) //5 second history size
        {
            out << getWordFmt(current[i]);
        }
    }

    out << "\n";

    bar_file.close();
}

//get word polybar formatted
QString BtCaptain::getWordFmt(BtHistory word)
{
    QString buf;

    if( word.is_final )
    {
        buf = "%{F#ddd}";
    }
    else
    {
        buf = "%{F#777}";
    }

    buf += "%{u";
    buf += getConfColor(word.conf);
    buf += "}%{+u}";

    int len = word.words.length();
    for( int i=0 ; i<len ; i++ )
    {
        buf += word.words[i];

        if( i<(len-1) )
        {
            buf += "->";
        }
    }

    buf += "%{-u} ";

    return buf;
}

float BtCaptain::getConf(BtWord word)
{
    int len = 100*(word.end - word.start);
    int gf = ENN_GAURD_TIME * 100; // gaurd_frame = 5
    float conf = net->getConf(word.stf-gf, len+2*gf, word.word_id);
    return conf;
}

QString BtCaptain::getConfColor(float conf)
{
    QColor color;
    if( conf>0.9 )
    {
        color = QColor::fromHsv(200, 200, 200);
    }
    else
    {
        color = QColor::fromHsv(conf*150, 200, 200);
    }
    return color.name();
}

void BtCaptain::flush()
{
    for( int i=0 ; i<current.length() ; i++ )
    {
        if( current[i].time>start_treshold ) //5 second history size
        {
            history.push_back(current[i]);
        }
        current.remove(i);
        i--;
    }
    BtHistory sep;
    sep.words.push_back(" ");
    if( history.length() )
    {
        BtHistory last = history.last();
        if( last.words[0]==" " )
        {
            sep.time = start_treshold+BT_HISTORY_SIZE/1000.0/2;
        }
        else
        {
            sep.time  = history.last().time;
        }
    }
    else
    {
        sep.time = start_treshold+BT_HISTORY_SIZE/1000.0/2;
    }
    sep.is_final = 1;
    sep.conf = 1;

    history.push_back(sep);
}
