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
#include "oneflow/user/kernels/pooling_kernel_util.h"

namespace oneflow {

std::vector<int32_t> Get3DVec(const std::vector<int32_t>& original_vec, int32_t NDims) {
  std::vector<int32_t> vec;
  FOR_RANGE(uint8_t, dim, 0, 3) {
    int64_t index = static_cast<int64_t>(dim) - (3 - NDims);
    if (index < 0) {
      vec.push_back(1);
    } else {
      vec.push_back(original_vec.at(index));
    }
  }
  return vec;
}

std::vector<int32_t> Get3DPadVec(const std::vector<int32_t>& original_vec, int32_t NDims) {
  std::vector<int32_t> vec;
  FOR_RANGE(uint8_t, dim, 0, 3) {
    int64_t index = static_cast<int64_t>(dim) - (3 - NDims);
    if (index < 0) {
      vec.push_back(0);
    } else {
      vec.push_back(original_vec.at(index));
    }
  }
  return vec;
}

void Get3DOutputShape(const DimVector& in, const std::vector<int32_t>& pool_size,
                      const std::vector<int32_t>& strides, const std::vector<int32_t>& padding,
                      const bool ceil_mode, std::vector<int32_t> dilation_rate, DimVector* out) {
  out->clear();
  out->resize(3);
  FOR_RANGE(size_t, i, 0, 3) {
    int64_t* out_ptr = &(*out).at(i);
    *out_ptr = (in.at(i) + 2 * padding.at(i) - dilation_rate.at(i) * (pool_size.at(i) - 1) - 1
                + strides.at(i) + (ceil_mode ? strides.at(i) - 1 : 0))
               / strides.at(i);
  }
}

PoolingParams3D::PoolingParams3D(const int32_t dim, const ShapeView& x_shape,
                                 const std::string& data_format,
                                 const std::vector<int32_t>& padding,
                                 const std::vector<int32_t>& kernel_size,
                                 const std::vector<int32_t>& stride,
                                 const std::vector<int32_t>& dilation, const bool return_indices,
                                 const bool ceil_mode)
    : dim_(dim),
      data_format_(data_format),
      padding_(Get3DPadVec(padding, dim)),
      pooling_size_3d_(Get3DVec(kernel_size, dim)),
      stride_3d_(Get3DVec(stride, dim)),
      dilation_3d_(Get3DVec(dilation, dim)),
      return_indices_(return_indices),
      ceil_mode_(ceil_mode) {
  x_3d_ = {GetInDim(x_shape, data_format, 0, dim), GetInDim(x_shape, data_format, 1, dim),
           GetInDim(x_shape, data_format, 2, dim)};
  Get3DOutputShape(x_3d_, pooling_size_3d_, stride_3d_, padding_, ceil_mode_, dilation_3d_, &y_3d_);
  if (data_format == "channels_first") {
    channel_num_ = x_shape.At(1);
  } else {
    CHECK_EQ(data_format_, "channels_last")
        << "data_format must be 'channels_first' or 'channels_last'";
    channel_num_ = x_shape.At(x_shape.NumAxes() - 1);
  }
  batch_num_ = x_shape.At(0);
}

void PoolingParams3D::Reset(const ShapeView& x_shape) {
  x_3d_ = {GetInDim(x_shape, data_format_, 0, dim_), GetInDim(x_shape, data_format_, 1, dim_),
           GetInDim(x_shape, data_format_, 2, dim_)};
  Get3DOutputShape(x_3d_, pooling_size_3d_, stride_3d_, padding_, ceil_mode_, dilation_3d_, &y_3d_);
}

Shape PoolingParams3D::GetYShape() const {
  DimVector y_dim_vec;
  if (dim_ == 1) {
    y_dim_vec = {y_3d_.at(2)};
  } else if (dim_ == 2) {
    y_dim_vec = {y_3d_.at(1), y_3d_.at(2)};
  } else if (dim_ == 3) {
    y_dim_vec = {y_3d_.at(0), y_3d_.at(1), y_3d_.at(2)};
  } else {
    UNIMPLEMENTED();
  }
  if (data_format_ == "channels_first") {
    y_dim_vec.insert(y_dim_vec.begin(), channel_num_);
  } else {
    CHECK_EQ(data_format_, "channels_last")
        << "data_format must be 'channels_first' or 'channels_last'";
    y_dim_vec.insert(y_dim_vec.end(), channel_num_);
  }
  y_dim_vec.insert(y_dim_vec.begin(), batch_num_);
  return Shape(y_dim_vec);
}

Shape PoolingParams3D::GetXShape5D() const {
  return Shape({batch_num_, channel_num_, x_3d_.at(0), x_3d_.at(1), x_3d_.at(2)});
}

Shape PoolingParams3D::GetYShape5D() const {
  return Shape({batch_num_, channel_num_, y_3d_.at(0), y_3d_.at(1), y_3d_.at(2)});
}

}  // namespace oneflow
