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
#ifndef ONEFLOW_CORE_VM_ONEFLOW_VM_H_
#define ONEFLOW_CORE_VM_ONEFLOW_VM_H_

#include "oneflow/core/common/notifier.h"
#include "oneflow/core/vm/interpret_type.h"
#include "oneflow/core/vm/vm_desc.h"
#include "oneflow/core/vm/virtual_machine_engine.h"
#include "oneflow/core/thread/thread_pool.h"

namespace oneflow {

class OneflowVM final {
 public:
  OneflowVM(const OneflowVM&) = delete;
  OneflowVM(OneflowVM&&) = delete;
  OneflowVM(const Resource& resource, int64_t this_machine_id);
  ~OneflowVM();

  Maybe<void> Receive(vm::InstructionMsgList* instr_list);

  const vm::VirtualMachineEngine& vm() const { return *vm_; }

 private:
  void Loop(const std::function<void()>& Initializer);

  vm::VirtualMachineEngine* mut_vm() { return vm_.Mutable(); }
  void ControlSync();

  intrusive::shared_ptr<vm::VirtualMachineEngine> vm_;
  // for asynchronized execution
  std::list<std::unique_ptr<std::thread>> worker_threads_;
  std::thread schedule_thread_;
  Notifier notifier_;
};

}  // namespace oneflow

#endif  // ONEFLOW_CORE_VM_ONEFLOW_VM_H_
