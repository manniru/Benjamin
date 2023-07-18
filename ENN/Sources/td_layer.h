#ifndef TD_LAYER_H
#define TD_LAYER_H

#include <algorithm>
#include <iomanip>
#include <limits>
#include <memory>
#include <numeric>
#include <queue>
#include <sstream>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>
#include <QObject>
#include <QDebug>

#include "tiny_dnn/core/backend.h"
#include "tiny_dnn/core/framework/device.fwd.h"
#include "tiny_dnn/node.h"

#include "tiny_dnn/util/parallel_for.h"
#include "tiny_dnn/util/product.h"
#include "tiny_dnn/util/util.h"
#include "tiny_dnn/util/weight_init.h"

#include "tiny_dnn/optimizers/optimizer.h"

/**
 * base class of all kind of NN layers
 *
 * sub-class should override these methods:
 * - forward_propagation ... body of forward-pass calculation
 * - back_propagation    ... body of backward-pass calculation
 * - inShape            ... specify input data shapes
 * - out_shape           ... specify output data shapes
 * - layer_type          ... name of layer
 **/
class TdLayer : public QObject, public tiny_dnn::node
{
    Q_OBJECT
public:
    explicit TdLayer(const std::vector<tiny_dnn::vector_type> &in_type,
                     const std::vector<tiny_dnn::vector_type> &out_type,
                     QObject *parent = nullptr);
    TdLayer(const TdLayer &) = default;
    TdLayer(TdLayer &&) = default;
    ~TdLayer() = default;

    void setParallelize(bool parallelize);
    void setBackend(std::shared_ptr<tiny_dnn::core::backend> b);
    void setBackendType(tiny_dnn::core::backend_t bt);
    void connection_mismatch(const TdLayer &from, const TdLayer &to);
    bool parallelize();
    tiny_dnn::core::backend_t getBackendType();
    tiny_dnn::core::backend_t getEngine();
    virtual std::string kernelFile();
    virtual std::string kernelHeader();
    virtual void createOp();
    void setDevice(const tiny_dnn::Device &device);
    tiny_dnn::Device *getDevice();
    std::shared_ptr<tiny_dnn::core::backend> getBackend();
    size_t inChannels();
    size_t outChannels();
    size_t inDataSize() const;
    size_t outDataSize() const;
    std::vector<tiny_dnn::shape3d> inDataShape();
    std::vector<tiny_dnn::shape3d> outDataShape();
    size_t inSize() const;
    size_t outSize() const;
    std::vector<const tiny_dnn::vec_t *> weights() const;
    std::vector<tiny_dnn::vec_t *> weights();
    std::vector<tiny_dnn::tensor_t *> weightsGrads();
    std::vector<tiny_dnn::edgeptr_t> inputs();
    std::vector<tiny_dnn::edgeptr_t> outputs();
    std::vector<tiny_dnn::edgeptr_t> outputs() const;
    void setOutGrads(const std::vector<const tiny_dnn::vec_t *> *grad, size_t cnt);
    void setInData(const std::vector<const tiny_dnn::vec_t *> *data, size_t cnt);
    void setInData(float *data, int len);
    void output(std::vector<const tiny_dnn::tensor_t *> &out) const;
    std::vector<tiny_dnn::vector_type> getInTypes() const;
    std::vector<tiny_dnn::vector_type> getOutTypes() const;
    void setTrainable(bool tr);
    bool getTrainable() const;

    virtual std::pair<float_t, float_t> outValueRange() const;
    /**
     * array of input shapes (width x height x depth)
     **/
    virtual std::vector<tiny_dnn::shape3d> inShape() const = 0;
    virtual void setInShape(const tiny_dnn::shape3d &in_shape);
    /**
     * array of output shapes (width x height x depth)
     **/
    virtual std::vector<tiny_dnn::shape3d> outShape() const = 0;
    /**
     * name of layer, should be unique for each concrete class
     **/
    virtual std::string layerType() const = 0;
    virtual size_t fanInSize() const;
    virtual size_t fanInSize(size_t) const;
    virtual size_t fan_out_size() const;
    virtual size_t fan_out_size(size_t) const;
    template <typename WeightInit>
    TdLayer &weightInit(const WeightInit &f);
    template <typename BiasInit>
    TdLayer &biasInit(const BiasInit &f);
    template <typename WeightInit>
    TdLayer &weightInit(std::shared_ptr<WeightInit> f);
    template <typename BiasInit>
    TdLayer &biasInit(std::shared_ptr<BiasInit> f);
    /**
     * @param in_data  input vectors of this layer (data, weight, bias)
     * @param out_data output vectors
     **/
    virtual void forwardPropagation(const std::vector<tiny_dnn::tensor_t *> &in_data,
                                     std::vector<tiny_dnn::tensor_t *> &out_data) = 0;
    /**
     * return delta of previous layer (delta=\frac{dE}{da}, a=wx in
     *fully-connected layer)
     * @param in_data  input vectors (same vectors as forward_propagation)
     * @param out_data output vectors (same vectors as forward_propagation)
     * @param out_grad gradient of output vectors (i-th vector correspond with
     *out_data[i])
     * @param in_grad  gradient of input vectors (i-th vector correspond with
     *in_data[i])
     **/
    virtual void backPropagation(const std::vector<tiny_dnn::tensor_t *> &in_data,
                                  const std::vector<tiny_dnn::tensor_t *> &out_data,
                                  std::vector<tiny_dnn::tensor_t *> &out_grad,
                                  std::vector<tiny_dnn::tensor_t *> &in_grad) = 0;
    virtual void postUpdate();
    virtual void setContext(tiny_dnn::net_phase ctx);
    std::vector<tiny_dnn::tensor_t> backward(
        const std::vector<tiny_dnn::tensor_t> &out_grads);
    void forward();
    void backward();
    void setup(bool reset_weight);
    void initWeight();
    void clearGrads();
    void updateWeight(tiny_dnn::optimizer *o);
    bool hasSameWeights(const TdLayer &rhs, float_t eps) const;
    virtual void setSampleCount(size_t sample_count);

    /**
     * generate layer from cereal's Archive
     **/
    template <typename InputArchive>
    static std::shared_ptr<TdLayer> load_layer(InputArchive &ia);

    template <typename OutputArchive>
    static void save_layer(OutputArchive &oa, const TdLayer &l);

    template <class Archive>
    void serialize_prolog(Archive &ar);

protected:
    template <typename T, typename Func>
    inline void for_i(T size, Func f, size_t grainsize = 100)
    {
      tiny_dnn::for_i(parallelized, size, f, grainsize);
    }

    /** Flag indication whether the layer/node is initialized */
    bool initialized;
    /** Flag indicating whether the layer/node operations ara paralellized */
    bool parallelized;
    /** The number of input vectors/edges */
    size_t in_channels;
    /** The number of output vectors/edges */
    size_t out_channels;
    /** Vector containing the type of data for inputs */
    std::vector<tiny_dnn::vector_type> in_type;
    /** Vector containing the type of data for outputs */
    std::vector<tiny_dnn::vector_type> out_type;
    /** The current backend type for operations */
    tiny_dnn::core::backend_t backend_type;
    /** The backend instance (deprecated) */
    std::shared_ptr<tiny_dnn::core::backend> backend;
    /** Pointer to the device on which the layer/node will run */
    tiny_dnn::Device *device_ptr = nullptr;
    /** Used in update_weight method. Kept as a member variable to reduce
     * frequent
     * memory allocation */
    tiny_dnn::vec_t weights_diff;

private:
    /* @brief Allocates the necessary edge memory in a specific
     * incoming connection.
     *
     * @param i The position to store the previous edge.
     *
     * Graphical explanation:
     *
     *     nullptr -- |edge| -- prev(i) ---- |layer|
     *               nullptr -- prev(i+1) -´
     */
    void alloc_input(size_t i) const
    {
      // the created incoming edge won't have a previous connection,
      // for this reason first parameter is a nullptr.
      prev_[i] = std::make_shared<tiny_dnn::edge>(nullptr, inShape()[i],
                                                  in_type[i]);
    }

    /* @brief Allocates the necessary edge memory in a specific
     * outcoming connection.
     *
     * @param i The position to store the next edge.
     *
     * Graphical explanation:
     *
     *     |layer| -- next(i) ---- |edge|
     *             `- next(i+1) -- nullptr
     */
    void alloc_output(size_t i) const
    {
      // the created outcoming will have the current layer as the
      // previous node.
      next_[i] = std::make_shared<tiny_dnn::edge>(const_cast<TdLayer *>(this),
                                                  outShape()[i], out_type[i]);
    }

    /* @brief Creates an edge between the current node and one incoming
     * or previous node.
     *
     * @param i The position to store the previous edge.
     *
     * The method checks if the edge already exists, otherwise we create it
     * and the necessary memory it's allocated. The method returns the pointer
     * to the previous edge.
     */
    tiny_dnn::edgeptr_t ith_in_node(size_t i)
    {
      // in case that the  edge doesn't exist, we create it
      if( !prev_[i] )
          alloc_input(i);
      return prev()[i];
    }

    /* @brief Creates an edge between the current node and one outcoming
     * or next node.
     *
     * @param i The position to store the next edge.
     *
     * The method checks if the edge already exists, otherwise we create it
     * and the necessary memory it's allocated. The method returns the pointer
     * to the next edge.
     */
    tiny_dnn::edgeptr_t ith_out_node(size_t i)
    {
      // in case that the  edge doesn't exist, we create it
      if (!next_[i]) alloc_output(i);
      return next()[i];
    }
    tiny_dnn::edgeptr_t ith_out_node(size_t i) const
    {
        return next()[i];
    }

    /* @brief Retrieves weight vector from incoming edge
     * @param i The position of incoming edge.
     *
     * Returns the mutable pointer to the edge raw data.
     */
    tiny_dnn::vec_t *getWeightData(size_t i)
    {
      assert(is_trainable_weight(in_type_[i]));
      return &(*(ith_in_node(i)->get_data()))[0];
    }

    /* @brief Retrieves weight vector from incoming edge
     * @param i The position of incoming edge.
     *
     * Returns the non mutable pointer to the edge raw data.
     */
    const tiny_dnn::vec_t *getWeightData(size_t i) const
    {
      assert(is_trainable_weight(in_type_[i]));
      return &(*(const_cast<TdLayer *>(this)->ith_in_node(i)->get_data()))[0];
    }

    /** Flag indicating whether the layer/node parameters are trainable */
    bool trainable;
    /** Pointer to the function for weights initialization */
    std::shared_ptr<tiny_dnn::weight_init::function> weight_init;
    /** Pointer to the function for biases initialization */
    std::shared_ptr<tiny_dnn::weight_init::function> bias_init;

    std::vector<tiny_dnn::tensor_t *> fwd_in_data;
    std::vector<tiny_dnn::tensor_t *> fwd_out_data;
    std::vector<tiny_dnn::tensor_t *> bwd_in_data;
    std::vector<tiny_dnn::tensor_t *> bwd_in_grad;
    std::vector<tiny_dnn::tensor_t *> bwd_out_data;
    std::vector<tiny_dnn::tensor_t *> bwd_out_grad;
};

#endif // TD_LAYER_H
