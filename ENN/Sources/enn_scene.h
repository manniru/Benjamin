#ifndef ENNSCENE_H
#define ENNSCENE_H

#include <QObject>
#include "backend.h"

#define SAMPLE_GRID_SIZE 10

class EnnScene : public QObject
{
    Q_OBJECT
public:
    explicit EnnScene(QObject *ui, QObject *parent = nullptr);

signals:

private slots:
    void loadFiles(QString word);
    void sampleAdded(int id);
    void updatePosition(int direction);

private:
    void initSampleGrid();
    void loadWordList();
    QString convertName(QString file_name);
    QString idToWord(int id);
    void writeSample(QString name, QString path, int w_id, int i);

    QVector<QObject *> sample_vector;
    QObject *root;            // root qml object
    QObject *sample_viewer;   //sample viewer qml object
    QObject *wordlist_panel;   //sample viewer qml object
    QString  enn_path;
    QVector<QString> files_list;
    QStringList word_list;
};

#endif // ENNSCENE_H
