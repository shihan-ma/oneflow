#ifndef ONEFLOW_CORE_VM_MEM_ZONE_TYPE_DESC_MSG_H_
#define ONEFLOW_CORE_VM_MEM_ZONE_TYPE_DESC_MSG_H_

#include "oneflow/core/vm/vpu_type_desc.msg.h"
#include "oneflow/core/vm/mem_zone_type_desc.msg.h"
#include "oneflow/core/vm/scheduler.msg.h"

namespace oneflow {

// clang-format off
BEGIN_OBJECT_MSG(VM);
  // fields
  OBJECT_MSG_DEFINE_OPTIONAL(VpuSchedulerCtx, vpu_scheduler_ctx); 
  // links
  OBJECT_MSG_DEFINE_SKIPLIST_HEAD(MemZoneTypeId, mem_zone_type_id, mem_zone_type_id2desc);
  OBJECT_MSG_DEFINE_SKIPLIST_HEAD(VpuTypeId, vpu_type_id, vpu_type_id2desc);
END_OBJECT_MSG(VM);
// clang-format on

}  // namespace oneflow

#endif  // ONEFLOW_CORE_VM_MEM_ZONE_TYPE_DESC_MSG_H_
