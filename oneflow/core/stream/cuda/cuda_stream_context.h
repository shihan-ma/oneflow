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
#ifndef ONEFLOW_CORE_STREAM_CUDA_CUDA_STREAM_CONTEXT_H_
#define ONEFLOW_CORE_STREAM_CUDA_CUDA_STREAM_CONTEXT_H_

#include "oneflow/core/stream/include/stream_context.h"
#include "oneflow/core/common/channel.h"
#include "oneflow/core/common/maybe.h"

#ifdef WITH_CUDA

#include "oneflow/core/device/cuda_util.h"

namespace oneflow {

class CudaStreamContext : public StreamContext {
 public:
  OF_DISALLOW_COPY_AND_MOVE(CudaStreamContext);
  CudaStreamContext() = default;
  ~CudaStreamContext() override = default;

  virtual cudaStream_t cuda_stream() const = 0;
  virtual cublasHandle_t cublas_handle() const = 0;
  virtual cudnnHandle_t cudnn_handle() const = 0;
};

}  // namespace oneflow

#endif

#endif  // ONEFLOW_CORE_STREAM_CUDA_CUDA_STREAM_CONTEXT_H_
