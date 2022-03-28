#include "kd_model.h"

using namespace kaldi;

KdModel::KdModel(std::string model_filename)
{
    oa_model = new AmDiagGmm;
    t_model = new TransitionModel;

    bool binary;
    Input ki(model_filename, &binary);
    t_model->Read(ki.Stream(), binary);
    oa_model->Read(ki.Stream(), binary);
}

KdModel::~KdModel()
{
    delete t_model;
    delete oa_model;
}
