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
#include "oneflow/core/common/switch_func.h"
#include "oneflow/core/framework/framework.h"
#include "oneflow/core/primitive/include/fill.h"

namespace oneflow {

namespace user_op {

namespace {

template<typename Context>
std::unique_ptr<primitive::Fill> NewFillPrimitive(Context* ctx) {
  const DataType data_type = ctx->TensorDesc4ArgNameAndIndex("out", 0)->data_type();
  return primitive::NewPrimitive<primitive::FillFactory>(ctx->device_type(), data_type);
}

class OnesLikeKernel final : public user_op::OpKernel {
 public:
  OnesLikeKernel() = default;
  ~OnesLikeKernel() = default;

 private:
  void Compute(user_op::KernelComputeContext* ctx) const override {
    user_op::Tensor* out = ctx->Tensor4ArgNameAndIndex("out", 0);
    std::unique_ptr<primitive::Fill> fill = primitive::NewPrimitive<primitive::FillFactory>(
        ctx->stream_ctx()->device_type(), out->data_type());
    CHECK(fill);
    fill->Launch(ctx->stream_ctx(), out->mut_dptr(), 1, out->shape().elem_cnt());
  }

  bool AlwaysComputeWhenAllOutputsEmpty() const override { return false; }
};

hob::HobContextGetter<user_op::KernelRegContext, bool> FillPrimitiveExists() {
  return user_op::HobCtxGetter<bool>(
      "FillPrimitiveExists",
      [](const user_op::KernelRegContext& ctx) { return NewFillPrimitive(&ctx).operator bool(); });
}

REGISTER_USER_KERNEL("ones_like")
    .SetCreateFn<OnesLikeKernel>()
    .SetIsMatchedHob(FillPrimitiveExists() == true);

}  // namespace

}  // namespace user_op

}  // namespace oneflow
