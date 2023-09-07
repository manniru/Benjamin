#ifndef TD_MINI_WORKER_H
#define TD_MINI_WORKER_H

#include <QObject>
#include "td_layer.h"
#include "td_nodes.h"
#include "tiny_dnn/lossfunctions/loss_function.h"

#define TD_THREAD_NUM 8

// Helper Class for Parallelization
class TdMiniWorker : public QObject
{
    Q_OBJECT
public:
    explicit TdMiniWorker(std::vector<TdLayer *> *nodes,
                      std::vector<tiny_dnn::tensor_t> *t_bat,
                      tiny_dnn::tensor_t *t_cobat,
                      QObject *parent = nullptr);

    std::vector<tiny_dnn::tensor_t> forward();
    void setRange(int s_id, int e_id);
    int enabled;

public slots:
    void run();

signals:
    void finished();

private:
    std::vector<TdLayer *> *nod;
    std::vector<tiny_dnn::tensor_t> *t_batch;
    tiny_dnn::tensor_t *t_cost_batch;
    int s_index;
    int e_index;
};

#endif // TD_MINI_WORKER_H
