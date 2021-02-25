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
#include "oneflow/core/operator/interface_op_util.h"
#include "oneflow/core/common/balanced_splitter.h"

namespace oneflow {

namespace {

void CheckShape(const Shape& shape) {
  FOR_RANGE(int, i, 1, shape.NumAxes()) { CHECK_GT(shape.At(i), 0); }
}

Maybe<void> GetSbpSignature(const InterfaceBlobConf& blob_conf, const PbRpf<std::string>& input_bns,
                            const PbRpf<std::string>& output_bns, SbpSignature* sbp_signature,
                            bool is_for_input_op) {
  if (blob_conf.split_axis().has_value()) {
    int64_t num_axes = blob_conf.shape().dim_size();
    int64_t split_axis = blob_conf.split_axis().value();
    if (split_axis < 0) { split_axis += num_axes; }
    CHECK_GE_OR_RETURN(split_axis, 0);
    CHECK_LT_OR_RETURN(split_axis, num_axes);

    SbpSignatureBuilder sbp_signature_builder;
    if (is_for_input_op) {
      // broadcast tick args for InputOp
      sbp_signature_builder.Broadcast(input_bns);
    } else {
      sbp_signature_builder.Split(input_bns, split_axis);
    }
    sbp_signature_builder.Split(output_bns, split_axis).Build(sbp_signature);
  } else {
    SbpSignatureBuilder().Broadcast(input_bns).Broadcast(output_bns).Build(sbp_signature);
  }
  return Maybe<void>::Ok();
}

}  // namespace

Maybe<void> InterfaceOpUtil::InferOutBlobDesc(const InterfaceBlobConf& blob_conf,
                                              BlobDesc* out_blob_desc,
                                              const ParallelContext* parallel_ctx) {
  out_blob_desc->mut_shape() = Shape(blob_conf.shape());
  CheckShape(out_blob_desc->shape());
  CHECK_GT(out_blob_desc->mut_shape().At(0), 0);
  out_blob_desc->set_data_type(blob_conf.data_type());
  out_blob_desc->set_is_dynamic(blob_conf.is_dynamic());
  out_blob_desc->set_is_tensor_list(blob_conf.is_tensor_list());
  // if (blob_conf.split_axis().has_value()) {
  //  int64_t split_axis = blob_conf.split_axis().value();
  //  BalancedSplitter bs(out_blob_desc->shape().At(split_axis), parallel_ctx->parallel_num());
  //  out_blob_desc->mut_shape().Set(split_axis, bs.At(parallel_ctx->parallel_id()).size());
  //}
  if (blob_conf.has_parallel_hierarchy()) {
    const Shape& parallel_hierarchy = Shape(blob_conf.parallel_hierarchy());
    if (parallel_hierarchy.NumAxes() == 1
        || (blob_conf.parallel_distribution().sbp_parallel(0)
            == blob_conf.parallel_distribution().sbp_parallel(1))) {
      if (blob_conf.parallel_distribution().sbp_parallel(0).has_split_parallel()) {
        int64_t split_axis =
            blob_conf.parallel_distribution().sbp_parallel(0).split_parallel().axis();
        BalancedSplitter bs(out_blob_desc->shape().At(split_axis), parallel_ctx->parallel_num());
        out_blob_desc->mut_shape().Set(split_axis, bs.At(parallel_ctx->parallel_id()).size());
      }
    } else {
      CHECK_EQ_OR_RETURN(parallel_hierarchy.NumAxes(), 2);
      int64_t rank_id_0 = parallel_ctx->parallel_id() / parallel_hierarchy.At(1);
      int64_t rank_id_1 = parallel_ctx->parallel_id() % parallel_hierarchy.At(1);
      if (blob_conf.parallel_distribution().sbp_parallel(0).has_split_parallel()) {
        int64_t split_axis =
            blob_conf.parallel_distribution().sbp_parallel(0).split_parallel().axis();
        BalancedSplitter bs(out_blob_desc->shape().At(split_axis), parallel_hierarchy.At(0));
        out_blob_desc->mut_shape().Set(split_axis, bs.At(rank_id_0).size());
      }
      if (blob_conf.parallel_distribution().sbp_parallel(1).has_split_parallel()) {
        int64_t split_axis =
            blob_conf.parallel_distribution().sbp_parallel(1).split_parallel().axis();
        BalancedSplitter bs(out_blob_desc->shape().At(split_axis), parallel_hierarchy.At(1));
        out_blob_desc->mut_shape().Set(split_axis, bs.At(rank_id_1).size());
      }
    }
  }
  LOG(INFO) << "out_blob_desc " << out_blob_desc->shape().ToString();
  return Maybe<void>::Ok();
}

Maybe<void> InterfaceOpUtil::GetInputLikeOpSbpSignature(const InterfaceBlobConf& blob_conf,
                                                        const PbRpf<std::string>& input_bns,
                                                        const PbRpf<std::string>& output_bns,
                                                        SbpSignature* sbp_signature) {
  GetSbpSignature(blob_conf, input_bns, output_bns, sbp_signature, true);
  return Maybe<void>::Ok();
}

Maybe<void> InterfaceOpUtil::GetOutputLikeOpSbpSignature(const InterfaceBlobConf& blob_conf,
                                                         const PbRpf<std::string>& input_bns,
                                                         const PbRpf<std::string>& output_bns,
                                                         SbpSignature* sbp_signature) {
  GetSbpSignature(blob_conf, input_bns, output_bns, sbp_signature, false);
  return Maybe<void>::Ok();
}

Maybe<void> InterfaceOpUtil::InitBlobConf(InterfaceBlobConf* blob_conf,
                                          const ParallelBlobConf& parallel_blob_conf) {
  LOG(INFO) << "parallel_blob_conf " << parallel_blob_conf.DebugString();
  BlobDesc blob_desc(parallel_blob_conf.logical_blob_desc_conf());
  blob_desc.shape().ToProto(blob_conf->mutable_shape());
  blob_conf->set_data_type(blob_desc.data_type());
  blob_conf->set_is_dynamic(blob_desc.is_dynamic());
  blob_conf->set_is_tensor_list(blob_desc.is_tensor_list());
  // TODO(liujuncheng): fully support
  // CHECK_EQ_OR_RETURN(parallel_blob_conf.parallel_distribution().sbp_parallel_size(), 1);
  const SbpParallel& sbp_parallel = parallel_blob_conf.parallel_distribution().sbp_parallel(0);
  if (sbp_parallel.has_split_parallel()) {
    int64_t axis = sbp_parallel.split_parallel().axis();
    blob_conf->mutable_split_axis()->set_value(axis);
  } else if (sbp_parallel.has_broadcast_parallel()) {
    blob_conf->mutable_split_axis()->clear_value();
  } else {
    OF_UNIMPLEMENTED();
  }
  *blob_conf->mutable_parallel_distribution() = parallel_blob_conf.parallel_distribution();
  *blob_conf->mutable_parallel_hierarchy() = parallel_blob_conf.parallel_hierarchy();
  return Maybe<void>::Ok();
}

}  // namespace oneflow
