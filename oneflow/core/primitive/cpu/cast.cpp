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
#include "oneflow/core/primitive/include/cast.h"
#include "oneflow/core/primitive/cpu/type_seq.h"

namespace oneflow {

namespace primitive {

namespace {

template<typename From, typename To>
void CastCpu(const From* from, To* to, size_t count) {
  for (size_t i = 0; i < count; ++i) { to[i] = static_cast<To>(from[i]); }
}

template<typename From, typename To>
class CastImpl : public Cast {
 public:
  OF_DISALLOW_COPY_AND_MOVE(CastImpl);
  CastImpl() = default;
  ~CastImpl() override = default;

  void Launch(StreamContext* stream_ctx, const void* from, void* to, size_t count) override {
    CastCpu(reinterpret_cast<const From*>(from), reinterpret_cast<To*>(to), count);
  }
};

template<typename From, typename To>
std::unique_ptr<Cast> NewCast() {
  return std::unique_ptr<Cast>(new CastImpl<From, To>());
}

class CastFactoryImpl : public CastFactory {
 public:
  OF_DISALLOW_COPY_AND_MOVE(CastFactoryImpl);
  CastFactoryImpl() = default;
  ~CastFactoryImpl() override = default;

  std::unique_ptr<Cast> New(DataType from, DataType to) override {
#define MAKE_NEW_CAST_ENTRY(from_pair, to_pair)                              \
  {std::make_pair(OF_PP_PAIR_SECOND(from_pair), OF_PP_PAIR_SECOND(to_pair)), \
   NewCast<OF_PP_PAIR_FIRST(from_pair), OF_PP_PAIR_FIRST(to_pair)>},

    static const std::map<std::pair<DataType, DataType>, std::function<std::unique_ptr<Cast>()>>
        new_cast_handle{OF_PP_SEQ_PRODUCT_FOR_EACH_TUPLE(
            MAKE_NEW_CAST_ENTRY, CPU_PRIMITIVE_ALL_TYPE_SEQ, CPU_PRIMITIVE_ALL_TYPE_SEQ)};

#undef MAKE_NEW_CAST_ENTRY

    const auto it = new_cast_handle.find(std::make_pair(from, to));
    if (it != new_cast_handle.end()) {
      return it->second();
    } else {
      return nullptr;
    }
  }
};

REGISTER_PRIMITIVE_FACTORY(DeviceType::kCPU, CastFactory, CastFactoryImpl);

}  // namespace

}  // namespace primitive

}  // namespace oneflow
