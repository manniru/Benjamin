#include "ab_ler_stat.h"
#include <QQmlProperty>

AbLerStat::AbLerStat(QObject *ui, QObject *parent) : QObject(parent)
{
    root = ui;
    ler_qml = root->findChild<QObject *>("LerStat");
    word_edit = root->findChild<QObject *>("TestStat");
    timer_editor = new QTimer();

    connect(root, SIGNAL(readTestErrors()),
            this, SLOT(loadLer()));
    connect(timer_editor, SIGNAL(timeout()),
            this, SLOT(timerTimeout()));
}

void AbLerStat::loadLer()
{
    readLerFile();
    clearEditor();
}

void AbLerStat::clearEditor()
{
    QMetaObject::invokeMethod(word_edit, "clearEditor");
    editor_lines.clear();
    // Qt QML destroy bug forced us to exert a timer
    // to make sure that the "clearEditor" function
    // made its clearance
    timer_editor->start(10);
}

void AbLerStat::timerTimeout()
{
    timer_editor->stop();
    int len = words.size();
    editor_lines.resize(len);
    for( int i=0 ; i<len ; i++ )
    {
        QString object_name = "WordLine" + QString::number(i);
        editor_lines[i] = word_edit->findChild<QObject *>(object_name);
        addWord(words[i], ler[i], "");
    }
}

void AbLerStat::addWord(QString word, int count, QString phoneme)
{
    QVariant word_v(word);
    QVariant phoneme_v(phoneme);

    QGenericArgument arg_word    = Q_ARG(QVariant, word_v);
    QGenericArgument arg_count   = Q_ARG(QVariant, count);
    QGenericArgument arg_phoneme = Q_ARG(QVariant, phoneme_v);

    QMetaObject::invokeMethod(word_edit, "addWord", arg_word,
                              arg_count, arg_phoneme);
}

void AbLerStat::readLerFile()
{
#ifdef WIN32
    QString filename = ab_getAudioPath();
    filename += + "..\\exp\\tri1\\decode\\scoring\\ler_stat";
#else //OR __linux
    QString filename = KAL_SCORE_DIR"ler_stat";
#endif
    QFile *ler_file = new QFile(filename);
    if( !ler_file->open(QIODevice::ReadOnly |
                        QIODevice::Text) )
    {
        qDebug() << "Error opening" << filename;
        return;
    }

    QString file_content = ler_file->readAll();
    QStringList lines = file_content.split("\n", Qt::SkipEmptyParts);
    int len = lines.size();

    int sum = 0;
    ler.resize(len);
    words.resize(len);
    for( int i=0 ; i<len ; i++ )
    {
        QStringList line_split = lines[i].split(" ");
        if( line_split.size()==2 )
        {
            bool convert_ok;
            ler[i] = line_split[1].toInt(&convert_ok);
            sum += ler[i];
            words[i] = line_split[0];

            if( !convert_ok )
            {
                qDebug() << "Warning: cannot parse line" << i << "from" << filename;
            }
        }
    }
    mean = sum/len;
    QQmlProperty::write(ler_qml, "mean", mean);
}

