#ifndef TD_NETWORK_H
#define TD_NETWORK_H

#include <QObject>
#ifndef CNN_NO_SERIALIZATION
#include <fstream>
#endif
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <limits>
#include <map>
#include <memory>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>
#include <QString>
#include <QDebug>

#include "tiny_dnn/lossfunctions/loss_function.h"
//#include "tiny_dnn/nodes.h"
#include "td_sequential.h"
#include "tiny_dnn/util/util.h"

class TdNetwork;

template <typename Layer>
TdNetwork &operator<<(TdNetwork &n, Layer &&l);

class TdNetwork : public QObject
{
    Q_OBJECT
public:
    explicit TdNetwork(QString name = "", QObject *parent = nullptr);

    void initWeight();
    void bprop(const std::vector<tiny_dnn::vec_t> &out,
               const std::vector<tiny_dnn::vec_t> &t,
               const std::vector<tiny_dnn::vec_t> &t_cost);
    void bprop(const std::vector<tiny_dnn::tensor_t> &out,
               const std::vector<tiny_dnn::tensor_t> &t,
               const std::vector<tiny_dnn::tensor_t> &t_cost);
    std::vector<tiny_dnn::vec_t> predict(const std::vector<tiny_dnn::vec_t> &in);
    std::vector<tiny_dnn::tensor_t> predict(const std::vector<tiny_dnn::tensor_t> &in);
    tiny_dnn::vec_t predict(const tiny_dnn::vec_t &in);
    void updateWeights(tiny_dnn::optimizer *opt);
    float_t predictMaxValue(const tiny_dnn::vec_t &in);
    tiny_dnn::label_t predictLabel(const tiny_dnn::vec_t &in);

    template <typename T, typename U>
    bool fit(tiny_dnn::adagrad &optimizer,
             const std::vector<T> &inputs,
             const std::vector<U> &desired_outputs,
             size_t batch_size,
             int epoch,
             const bool reset_weights     = false,
             const int n_threads          = CNN_TASK_SIZE,
             const std::vector<U> &t_cost = std::vector<U>());
    template <typename T, typename U>
    bool fit(tiny_dnn::adagrad &optimizer,
             const std::vector<T> &inputs,
             const std::vector<U> &desired_outputs,
             size_t batch_size = 1,
             int epoch         = 1);
    void setNetPhase(tiny_dnn::net_phase phase);
    void stopOngoingTraining();
    std::vector<tiny_dnn::vec_t> test(
            const std::vector<tiny_dnn::vec_t> &in);
    float_t getLoss(const std::vector<tiny_dnn::vec_t> &in,
                     const std::vector<tiny_dnn::label_t> &t);
    float_t getLoss(const std::vector<tiny_dnn::vec_t> &in,
                     const std::vector<tiny_dnn::vec_t> &t);
    template <typename T>
    float_t getLoss(const std::vector<T> &in,
                    const std::vector<tiny_dnn::tensor_t> &t);
    size_t layerSize();
    size_t depth();
    tiny_dnn::layer *operator[](size_t index);
    template <typename T> T &at(size_t index);
    size_t outDataSize();
    size_t inDataSize();
    template <typename WeightInit>
    TdNetwork &weightInit(const WeightInit &f);
    template <typename BiasInit>
    TdNetwork &biasInit(const BiasInit &f);
    void load(const std::string &filename);
    void save(const std::string &filename);
    void toArchive(cereal::BinaryOutputArchive &ar);
    void fromArchive(cereal::BinaryInputArchive &ar);
    bool fit(tiny_dnn::adagrad &optimizer,
             const std::vector<tiny_dnn::tensor_t> &inputs,
             const std::vector<tiny_dnn::tensor_t> &desired_outputs,
             size_t batch_size,
             int epoch,
             const bool reset_weights            = false,
             const int n_threads                 = CNN_TASK_SIZE,
             const std::vector<tiny_dnn::tensor_t> &t_cost =
                   std::vector<tiny_dnn::tensor_t>());
    void normalizeTensor(const std::vector<tiny_dnn::tensor_t> &inputs,
                         std::vector<tiny_dnn::tensor_t> &normalized);
    void normalizeTensor(const std::vector<tiny_dnn::vec_t> &inputs,
                         std::vector<tiny_dnn::tensor_t> &normalized);
    void normalizeTensor(const std::vector<tiny_dnn::label_t> &inputs,
                          std::vector<tiny_dnn::tensor_t> &normalized);

    TdSequential net;
    typedef typename std::vector<tiny_dnn::layer *>::iterator iterator;
    typedef typename std::vector<tiny_dnn::layer *>::const_iterator const_iterator;
    std::vector<tiny_dnn::tensor_t> in_batch;
    std::vector<tiny_dnn::tensor_t> t_batch;
    QString net_name;
    int     stop_training;

signals:
    void onBatchEnumerate();
    void OnEpochEnumerate();

protected:
    float_t fprop_max(const tiny_dnn::vec_t &in);

    tiny_dnn::label_t fprop_max_index(const tiny_dnn::vec_t &in);

    template <typename Layer>
    friend TdNetwork &operator<<(TdNetwork &n, Layer &&l);

private:
    void train_once(tiny_dnn::adagrad &optimizer,
                    const tiny_dnn::tensor_t *in,
                    const tiny_dnn::tensor_t *t,
                    int size,
                    const int nbThreads,
                    const tiny_dnn::tensor_t *t_cost);
    void train_onebatch(tiny_dnn::adagrad &optimizer,
                        const tiny_dnn::tensor_t *in,
                        const tiny_dnn::tensor_t *t,
                        int batch_size,
                        const int num_tasks,
                        const tiny_dnn::tensor_t *t_cost);
    bool calcDelta(const std::vector<tiny_dnn::tensor_t> &in,
                    const std::vector<tiny_dnn::tensor_t> &v,
                    tiny_dnn::vec_t &w,
                    tiny_dnn::tensor_t &dw,
                    size_t check_index,
                    double eps);
    void checkTargetCostMatrix(const std::vector<tiny_dnn::tensor_t> &t,
                               const std::vector<tiny_dnn::tensor_t> &t_cost);
    void checkTargetCostElement(const tiny_dnn::vec_t &t,
                                const tiny_dnn::vec_t &t_cost);
    void checkTargetCostElement(const tiny_dnn::tensor_t &t,
                                const tiny_dnn::tensor_t &t_cost);
    const tiny_dnn::tensor_t *getTargetCostSamplePointer(
            const std::vector<tiny_dnn::tensor_t> &t_cost, size_t i);
};

template <typename Layer>
TdNetwork &operator <<(TdNetwork &n, Layer &&l)
{
    n.net.add(std::forward<Layer>(l));
    return n;
}

template <typename Char, typename CharTraits>
std::basic_ostream<Char, CharTraits> &operator<<(
        std::basic_ostream<Char, CharTraits> &os, const TdNetwork &n)
{
    n.save(os);
    return os;
}

template <typename Char, typename CharTraits>
std::basic_istream<Char, CharTraits> &operator>>(
        std::basic_istream<Char, CharTraits> &os, TdNetwork &n)
{
    n.load(os);
    return os;
}


#endif // TD_NETWORK_H
