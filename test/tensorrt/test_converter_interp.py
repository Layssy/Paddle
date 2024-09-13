# Copyright (c) 2024 PaddlePaddle Authors. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import unittest

import numpy as np
from tensorrt_test_base import TensorRTBaseTest

import paddle


def upsample_wrapper(x):
    upsample = paddle.nn.Upsample(size=[12, 12])
    return upsample(x)


class TestFlattenTRTPattern(TensorRTBaseTest):
    def setUp(self):
        self.python_api = upsample_wrapper
        self.api_args = {"x": np.random.random([2, 3, 6, 10]).astype("float32")}
        self.program_config = {"feed_list": ["x"]}
        self.min_shape = {"x": [2, 3, 6, 10]}
        self.max_shape = {"x": [12, 3, 6, 10]}

    def test_trt_result(self):
        self.check_trt_result()


if __name__ == "__main__":
    unittest.main()
