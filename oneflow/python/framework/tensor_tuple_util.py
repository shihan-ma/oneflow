"""
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
"""

import collections
from typing import Union, Sequence, Tuple, Optional

from oneflow._oneflow_internal import TensorTuple, Tensor


def convert_to_tensor_tuple(args: Optional[Union[Tensor, Sequence[Tensor]]]):
    if args is None:
        return TensorTuple()
    elif isinstance(args, collections.abc.Sequence):
        return TensorTuple(args)
    else:
        tensor_tuple = TensorTuple()
        tensor_tuple.append(args)
        return tensor_tuple
