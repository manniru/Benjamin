#ifndef TD_MINI_WORKER_H
#define TD_MINI_WORKER_H

#include <QObject>
#include "td_layer.h"
#include "td_nodes.h"
#include "backend.h"
#include "tiny_dnn/lossfunctions/loss_function.h"

#define TD_THREAD_NUM 8

// Helper Class for Parallelization
class TdMiniWorker : public QObject
{
    Q_OBJECT
public:
    explicit TdMiniWorker(std::vector<TdLayer *> *nodes,
                      std::vector<int> *o,
                      std::vector<float> *t_co,
                      QObject *parent = nullptr);

    std::vector<tiny_dnn::tensor_t> forward();
    void setRange(int s_id, int e_id);
    int enabled;
    std::vector<tiny_dnn::tensor_t> calcGrad(
           std::vector<tiny_dnn::tensor_t> &y);
    void applyCost(tiny_dnn::vec_t &calcGrad);
    tiny_dnn::vec_t mse_df(tiny_dnn::vec_t &y, float o);

public slots:
    void run();

signals:
    void finished();

private:
    std::vector<TdLayer *> *nod;
    std::vector<int> *outputs;
    std::vector<float> *costs;
    int s_index;
    int e_index;
    clock_t start_time;
};

#endif // TD_MINI_WORKER_H
