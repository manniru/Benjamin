#ifndef TD_WORKER_H
#define TD_WORKER_H

#include <QThread>
#include <QGuiApplication>
#include "td_mini_worker.h"
#include "tiny_dnn/optimizers/optimizer.h"

// Helper Class for Parallelization
class TdWorker : public QObject
{
    Q_OBJECT
public:
    explicit TdWorker(std::vector<TdLayer *> *nodes,
                      QObject *parent = nullptr);

    void fit(tiny_dnn::tensor_t *inputs,
                    std::vector<tiny_dnn::tensor_t> *desired_outputs,
                    int epoch,
                    tiny_dnn::tensor_t *t_cost);
    void trainMiniBatch(int data_size,
                               int offset);
    void setBatchSize(int bs);
    int runningWorkersNum();
    void trainEpoch();
    void trainBatch();

    QVector<TdMiniWorker *> workers;
    QVector<QThread *>      workers_th;
    tiny_dnn::adagrad       optimizer;
    std::vector<tiny_dnn::tensor_t> t_batch;
    tiny_dnn::tensor_t t_cost_batch;
    int stop_training;

public slots:
    void miniWorkerFinished();

signals:
    void startMiniWorkers();
    void onEpochEnumerate();
    void epochFinished();

private:
    std::vector<TdLayer *> *nod;
    int batch_size;
    int worker_len;
    int worker_done;

    int iter_cnt;
    int batch_cnt;
    int total_epoch;
    tiny_dnn::tensor_t *in_vec;
    std::vector<tiny_dnn::tensor_t> *outputs;
    tiny_dnn::tensor_t *cost;
};

#endif // TD_WORKER_H
