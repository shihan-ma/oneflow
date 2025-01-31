/*
Copyright 2020 The OneFlow Authors. All rights reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/
#ifndef ONEFLOW_CORE_PRIMITIVE_BROADCAST_MATMUL_H_
#define ONEFLOW_CORE_PRIMITIVE_BROADCAST_MATMUL_H_

#include "oneflow/core/primitive/include/primitive.h"
#include "oneflow/core/primitive/include/blas.h"
#include "oneflow/core/common/scalar.h"

namespace oneflow {

namespace primitive {

class BroadcastMatmul : public Primitive {
 public:
  OF_DISALLOW_COPY_AND_MOVE(BroadcastMatmul);
  BroadcastMatmul() = default;
  ~BroadcastMatmul() override = default;

  virtual void Launch(StreamContext* stream_ctx, Scalar alpha, size_t num_a_dims,
                      const int64_t* a_dims, const void* a, size_t num_b_dims,
                      const int64_t* b_dims, const void* b, Scalar beta, size_t num_c_dims,
                      const int64_t* c_dims, void* c) = 0;
};

class BroadcastMatmulFactory : public Factory<BroadcastMatmul> {
 public:
  OF_DISALLOW_COPY_AND_MOVE(BroadcastMatmulFactory);
  BroadcastMatmulFactory() = default;
  ~BroadcastMatmulFactory() override = default;

  virtual std::unique_ptr<BroadcastMatmul> New(DataType data_type, BlasTransposeType transpose_a,
                                               BlasTransposeType transpose_b,
                                               size_t max_num_dims) = 0;
};

}  // namespace primitive

}  // namespace oneflow

#endif  // ONEFLOW_CORE_PRIMITIVE_BROADCAST_MATMUL_H_
