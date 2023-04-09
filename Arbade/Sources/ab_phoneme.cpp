#include "ab_phoneme.h"
#include <fcntl.h>
#include <unistd.h>

AbPhoneme::AbPhoneme(QObject *parent): QObject(parent)
{
    loadPhoneme();
    getPhoneme("norbert");
}

AbPhoneme::~AbPhoneme()
{

}

QString AbPhoneme::getPhoneme(QString word)
{
    std::binary_search(words.begin(), words.end(), word);
    QVector<QString>::iterator low_val = std::lower_bound(
                words.begin(), words.end(), word);

    if( low_val==words.end() || *low_val!=word )
    {
        return "";
    }
    else
    {
        int index = low_val-words.begin();
        return phonemes[index];
    }
}

void AbPhoneme::loadPhoneme()
{
    QString filename = ab_getAudioPath();
    filename += + "..\\scripts\\lang\\cmudict.dict";
    QFile words_file(filename);

    if( !words_file.open(QIODevice::ReadOnly | QIODevice::Text) )
    {
        qDebug() << "Error opening" << filename;
        return;
    }

    while( !words_file.atEnd() )
    {
        QString line = QString(words_file.readLine());
        line = line.trimmed();
        QStringList line_list = line.split(" ");
        words << line_list[0];

        int len = line_list.length();
        QString phones;
        for( int i=1 ; i<len ; i++ )
        {
            phones += line_list[i] + " ";
        }
        phones = phones.trimmed();
        phonemes << phones;
    }

    words_file.close();
}
