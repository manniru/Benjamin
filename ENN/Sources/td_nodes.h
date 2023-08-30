#ifndef TD_NODES_H
#define TD_NODES_H

#include <memory>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>
#include <QDebug>

#include "tiny_dnn/optimizers/optimizer.h"
#include "tiny_dnn/util/util.h"
#include "td_layer.h"

void td_isSizeMatch(std::vector<TdLayer *> *nod);

void td_connectHeadToTail(std::vector<TdLayer *> *nod);

void td_reorder(
    const std::vector<tiny_dnn::tensor_t> &input,
    std::vector<std::vector<const tiny_dnn::vec_t *>> &output);

std::vector<tiny_dnn::tensor_t> td_normalizeOut(
  const std::vector<const tiny_dnn::tensor_t *> &out);

#endif // TD_NODES_H
