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
#include "oneflow/core/eager/local_call_opkernel_phy_instr_operand.h"
#include "oneflow/user/kernels/stateful_local_opkernel.h"
#include "oneflow/core/eager/dev_vm_dep_object_consume_mode.h"

namespace oneflow {
namespace vm {

void LocalCallOpKernelPhyInstrOperand::ForEachConstMirroredObject(
    const std::function<void(vm::MirroredObject* infer, vm::MirroredObject* compute)>& DoEach)
    const {
  const auto& input_list = inputs();
  for (int64_t index : opkernel().input_tuple_indexes4const_ibns()) {
    const auto& input = input_list->at(index);
    DoEach(nullptr, CHECK_JUST(input->compute_local_dep_object())->mut_mirrored_object());
  }
}

void LocalCallOpKernelPhyInstrOperand::ForEachMutMirroredObject(
    const std::function<void(vm::MirroredObject* infer, vm::MirroredObject* compute)>& DoEach)
    const {
  const auto& device = opkernel().device();
  const auto& opt_transport_dep_object = device->mut_transport_local_dep_object();
  if (opt_transport_dep_object.has_value()) {
    DoEach(nullptr, CHECK_JUST(opt_transport_dep_object)->mut_mirrored_object());
  }
  auto* device_schedule_dep_object = device->mut_schedule_local_dep_object();
  if (device->type() == "async_launched_nccl") {
    // Sequantialize nccl instructions to avoid deadlock
    DoEach(nullptr, device_schedule_dep_object->mut_mirrored_object());
  } else {
    // Sequantialize instructions to avoid explosive memory allocation of source ops
    if (dev_vm_dep_object_consume_mode() == one::DevVmDepObjectConsumeMode::MUTABLE) {
      DoEach(nullptr, device_schedule_dep_object->mut_mirrored_object());
    }
  }

  const auto& input_list = inputs();
  for (int64_t index : opkernel().input_tuple_indexes4mut_ibns()) {
    const auto& input = input_list->at(index);
    DoEach(nullptr, CHECK_JUST(input->compute_local_dep_object())->mut_mirrored_object());
  }
  const auto& output_list = outputs();
  for (int64_t index : opkernel().output_tuple_indexes4mut_obns()) {
    const auto& output = output_list->at(index);
    DoEach(nullptr, CHECK_JUST(output->compute_local_dep_object())->mut_mirrored_object());
  }
}

void LocalCallOpKernelPhyInstrOperand::ForEachMut2MirroredObject(
    const std::function<void(vm::MirroredObject* infer, vm::MirroredObject* compute)>& DoEach)
    const {
  const auto& output_list = outputs();
  for (int64_t index : opkernel().output_tuple_indexes4mut2_obns()) {
    const auto& output = output_list->at(index);
    DoEach(nullptr, CHECK_JUST(output->compute_local_dep_object())->mut_mirrored_object());
  }
}

DTRInstrOperand::DTRInstrOperand(
      const std::shared_ptr<one::StatefulLocalOpKernel>& opkernel,
      const one::EagerBlobObjectListPtr& input, const one::EagerBlobObjectListPtr& output,
      const std::shared_ptr<const one::ConsistentTensorInferResult>& consistent_tensor_infer_result,
      const one::OpExprInterpContext& op_interp_ctx_,
      const one::DevVmDepObjectConsumeMode dev_vm_dep_object_consume_mode)
      : opkernel_(opkernel),
        consistent_tensor_infer_result_(consistent_tensor_infer_result),
        op_interp_ctx_(op_interp_ctx_),
        dev_vm_dep_object_consume_mode_(dev_vm_dep_object_consume_mode) {
          for (const auto &x : *input) {
            for (const auto &y : *output) {
              if (x.get() == y.get()) {
                std::cout << "inplace!!!!" << std::endl;
                std::cout << opkernel->user_op_conf_->op_type_name() << std::endl;
              }
            }
          }
    // inputs & outputs weak_ptr
    inputs_ = std::vector<std::weak_ptr<vm::EagerBlobObject>>();
    for (const auto& in : *input) { inputs_.emplace_back(in); }
    outputs_ = std::vector<std::weak_ptr<vm::EagerBlobObject>>();
    for (const auto& out : *output) { outputs_.emplace_back(out); }
  }
}  // namespace vm
}  // namespace oneflow
