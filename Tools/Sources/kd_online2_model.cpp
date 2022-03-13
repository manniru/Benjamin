#include "kd_online2_model.h"

using namespace kaldi;
using namespace fst;

KdOnline2Model::KdOnline2Model(TransitionModel *tran, AmDiagGmm *acc_model,
                               std::string model_filename)
{
    t_model = tran;
    a_model = acc_model;

    bool binary;
    Input ki(model_filename, &binary);
    TransitionModel tmodel;
    tmodel.Read(ki.Stream(), binary);
    if( !tmodel.Compatible(*t_model))
        KALDI_ERR << "Incompatible models given to the --model and "
                  << "--online-alignment-model options";
    oa_model = new AmDiagGmm;
    oa_model->Read(ki.Stream(), binary);
}


TransitionModel *KdOnline2Model::GetTransitionModel()
{
    return t_model;
}

AmDiagGmm *KdOnline2Model::GetOnlineAlignmentModel()
{
    if( oa_model->NumPdfs()!=0 )
    {
        return oa_model;
    }
    else
    {
        return a_model;
    }
}

AmDiagGmm *KdOnline2Model::GetModel()
{
    return a_model;
}

AmDiagGmm *KdOnline2Model::GetFinalModel()
{
    return a_model;
}

