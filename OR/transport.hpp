#ifndef TRANSPORT_H
#define TRANSPORT_H

#include "share.hpp"
#include <Eigen/Core>

namespace OR {
void transportSimplexMethodMax(DynamicMatrix &cost,
                               Eigen::VectorX<float> &supply,
                               Eigen::RowVectorX<float> &demand,
                               DynamicMatrix &solution, float &minmax);
}

#endif // TRANSPORT_H