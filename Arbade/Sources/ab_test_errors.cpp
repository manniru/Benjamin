#include "ab_test_errors.h"

AbTestErrors::AbTestErrors(QObject *ui, QObject *parent) : QObject(parent)
{
    root = ui;
    t_err_qml = root->findChild<QObject *>("TestErrors");
    t_word_edit = root->findChild<QObject *>("TestStat");
    timer_editor = new QTimer();

    connect(root, SIGNAL(readTestErrors()),
            this, SLOT(loadWordErrors()));
    connect(timer_editor, SIGNAL(timeout()),
            this, SLOT(timerTimeout()));
}

void AbTestErrors::loadWordErrors()
{
    readWordErrors();
    clearEditor();
}

void AbTestErrors::clearEditor()
{
    QMetaObject::invokeMethod(t_word_edit, "clearEditor");
    editor_lines.clear();
    // Qt QML destroy bug forced us to exert a timer
    // to make sure that the "clearEditor" function
    // made its clearance
    timer_editor->start(10);
}

void AbTestErrors::timerTimeout()
{
    timer_editor->stop();
    int len = words.size();
    editor_lines.resize(len);
    for( int i=0 ; i<len ; i++ )
    {
        QString object_name = "WordLine" + QString::number(i);
        editor_lines[i] = t_word_edit->findChild<QObject *>(object_name);
        addWord(words[i], word_errors[i], "");
    }
}

void AbTestErrors::addWord(QString word, int count, QString phoneme)
{
    QVariant word_v(word);
    QVariant phoneme_v(phoneme);

    QGenericArgument arg_word    = Q_ARG(QVariant, word_v);
    QGenericArgument arg_count   = Q_ARG(QVariant, count);
    QGenericArgument arg_phoneme = Q_ARG(QVariant, phoneme_v);

    QMetaObject::invokeMethod(t_word_edit, "addWord", arg_word,
                              arg_count, arg_phoneme);
}

void AbTestErrors::readWordErrors()
{
#ifdef WIN32
    QString filename = ab_getAudioPath();
    filename += + "..\\exp\\tri1\\decode\\scoring\\test_errors";
#else //OR __linux
    QString filename = KAL_SCORE_DIR"test_errors";
#endif
    QFile *t_err_file = new QFile(filename);
    if( !t_err_file->open(QIODevice::ReadOnly |
                        QIODevice::Text) )
    {
        qDebug() << "Error opening" << filename;
        return;
    }

    QString file_content = t_err_file->readAll();
    QStringList lines = file_content.split("\n");
    int len = lines.size();

    word_errors.resize(len);
    words.resize(len);
    for( int i=0 ; i<len ; i++ )
    {
        QStringList line_split = lines[i].split(" ");
        if( line_split.size()==2 )
        {
            bool convert_ok;
            word_errors[i] = line_split[1].toInt(&convert_ok);
            words[i] = line_split[0];

            if( !convert_ok )
            {
                qDebug() << "Warning: cannot parse line" << i << "from" << filename;
            }
        }
    }
}
