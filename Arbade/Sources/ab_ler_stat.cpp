#include "ab_ler_stat.h"
#include <QQmlProperty>
#include <algorithm>

AbLerStat::AbLerStat(QObject *ui, QObject *parent) : QObject(parent)
{
    root = ui;
    ler_qml = root->findChild<QObject *>("LerStat");
    timer_editor = new QTimer();

    connect(root, SIGNAL(readLerDiff()),
            this, SLOT(loadLer()));
    connect(timer_editor, SIGNAL(timeout()),
            this, SLOT(timerTimeout()));
}

void AbLerStat::loadLer()
{
    resetLerVars();
    readLerFile();
    updateLerMean();
    sortLer();
    clearEditor(); // after clearing editor, all stat will be displayed
}

void AbLerStat::clearEditor()
{
    QMetaObject::invokeMethod(ler_qml, "clearEditor");
    // Qt QML destroy bug forced us to exert a timer
    // to make sure that the "clearEditor" function
    // made its clearance
    timer_editor->start(10);
}

void AbLerStat::timerTimeout()
{
    timer_editor->stop();
    int len = wrong_count.size();

    for( int k=0 ; k<len ; k++ )
    {
        int i = sorted_indices[k];
        int len_wrong = wrong_out[i].size();
        for( int j=0 ; j<len_wrong ; j++ )
        {
            int m = sorted_wr_indices[i][j];
            QString wrong_str = wrong_out[i][m] + " " +
                    QString::number(wrong_count[i][m]);
            if( j==0 )
            {
                addWord(words[i], QString::number(ler[i]), wrong_str);
            }
            else
            {
                addWord("", "", wrong_str);
            }
        }
    }
    addWord(QString::number(sum_ler), "Sum", "");
}

void AbLerStat::addWord(QString word, QString count, QString wrong)
{
    QVariant word_v(word);
    QVariant count_v(count);
    QVariant wrong_v(wrong);

    QGenericArgument arg_word    = Q_ARG(QVariant, word_v);
    QGenericArgument arg_count   = Q_ARG(QVariant, count_v);
    QGenericArgument arg_wrong = Q_ARG(QVariant, wrong_v);

    QMetaObject::invokeMethod(ler_qml, "addWord", arg_word,
                              arg_count, arg_wrong);
}

void AbLerStat::readLerFile()
{
#ifdef WIN32
    QString filename = ab_getAudioPath();
    filename += + "..\\exp\\tri1\\decode\\scoring\\ler_diff";
#else //OR __linux
    QString filename = KAL_SCORE_DIR"ler_diff";
#endif
    QFile *ler_file = new QFile(filename);
    if( !ler_file->open(QIODevice::ReadOnly |
                        QIODevice::Text) )
    {
        qDebug() << "Error opening" << filename;
        return;
    }

    QString file_content = ler_file->readAll();
    QStringList lines = file_content.split("\n", QString::SkipEmptyParts);
    int len = lines.size();

    for( int i=0 ; i<len ; i++ )
    {
        int reverse_order = QQmlProperty::read(ler_qml, "reverse_order").toInt();
        QStringList line_split = lines[i].split("->");
        if( line_split.size()==2 )
        {
            QString word1, word2;
            if( reverse_order )
            {
                word1 = line_split[1];
                word2 = line_split[0];
            }
            else
            {
                word1 = line_split[0];
                word2 = line_split[1];
            }
            addToLerStat(word1, word2);
        }
    }
}

void AbLerStat::addToLerStat(QString word1, QString word2)
{
    if( words.contains(word1) )
    {
        int w_index = words.indexOf(word1);
        ler[w_index]++;
        if( !wrong_out[w_index].contains(word2) )
        {
            wrong_out[w_index].append(word2);
            wrong_count[w_index].append(1);
        }
        else
        {
            int w2_index = wrong_out[w_index].indexOf(word2);
            wrong_count[w_index][w2_index]++;
        }
    }
    else
    {
        words.append(word1);
        ler.append(1);
        wrong_out.append(QStringList(word2));
        wrong_count.append(QVector<int>(1, 1));
    }
}

void AbLerStat::updateLerMean()
{
    int len = ler.size();
    sum_ler = 0;
    for( int i=0 ; i<len ; i++ )
    {
        sum_ler += ler[i];
    }
    mean = sum_ler/len;

    QQmlProperty::write(ler_qml, "mean", mean);
}

void AbLerStat::sortLer()
{
    sorted_indices = sortAndGetIndices(ler);
    int len = words.length();
    sorted_wr_indices.resize(len);
    for( int i=0 ; i<len ; i++ )
    {
        sorted_wr_indices[i] = sortAndGetIndices(wrong_count[i]);
    }
}

// clear ler vectors
void AbLerStat::resetLerVars()
{
    ler.clear();
    words.clear();
    wrong_out.clear();
    wrong_count.clear();
}

QVector<int> sortAndGetIndices(const QVector<int>& data)
{
    // Create a QVector of indices [0, 1, 2, ..., n-1]
    QVector<int> indices(data.size());
    std::iota(indices.begin(), indices.end(), 0);

    // Define a custom comparator function
    auto comparator = [&data](int i, int j)
    {
        return data[i] > data[j];
    };

    // Sort the indices vector based on the values in the data vector
    std::sort(indices.begin(), indices.end(), comparator);

    return indices;
}
