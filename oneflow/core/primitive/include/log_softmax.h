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
#ifndef ONEFLOW_CORE_PRIMITIVE_LOG_SOFTMAX_H_
#define ONEFLOW_CORE_PRIMITIVE_LOG_SOFTMAX_H_

#include "oneflow/core/primitive/include/primitive.h"

namespace oneflow {

namespace primitive {

class LogSoftmax : public Primitive {
 public:
  OF_DISALLOW_COPY_AND_MOVE(LogSoftmax);
  LogSoftmax() = default;
  ~LogSoftmax() override = default;

  virtual void Launch(StreamContext* stream_ctx, size_t rows, size_t cols, const void* x,
                      void* y) = 0;
};

class LogSoftmaxFactory : public Factory<LogSoftmax> {
 public:
  OF_DISALLOW_COPY_AND_MOVE(LogSoftmaxFactory);
  LogSoftmaxFactory() = default;
  ~LogSoftmaxFactory() override = default;

  virtual std::unique_ptr<LogSoftmax> New(DataType data_type) = 0;
};

}  // namespace primitive

}  // namespace oneflow

#endif  // ONEFLOW_CORE_PRIMITIVE_LOG_SOFTMAX_H_
