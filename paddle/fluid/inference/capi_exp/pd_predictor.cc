// Copyright (c) 2021 PaddlePaddle Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "paddle/fluid/inference/capi_exp/pd_predictor.h"

#include "paddle/fluid/inference/api/paddle_inference_api.h"
#include "paddle/fluid/inference/capi_exp/pd_config.h"
#include "paddle/fluid/inference/capi_exp/pd_types.h"
#include "paddle/fluid/inference/capi_exp/pd_utils.h"
#include "paddle/fluid/inference/capi_exp/types_internal.h"
#include "paddle/fluid/inference/capi_exp/utils_internal.h"
#include "paddle/fluid/platform/enforce.h"

#define CHECK_AND_CONVERT_PD_PREDICTOR                              \
  PADDLE_ENFORCE_NOT_NULL(                                          \
      pd_predictor,                                                 \
      phi::errors::InvalidArgument(                                 \
          "The pointer of paddle predictor shouldn't be nullptr")); \
  auto& predictor = pd_predictor->predictor

extern "C" {
__pd_give PD_Predictor* PD_PredictorCreate(__pd_take PD_Config* pd_config) {
  PADDLE_ENFORCE_NOT_NULL(
      pd_config,
      phi::errors::InvalidArgument(
          "The pointer of paddle predictor shouldn't be nullptr"));
  PD_Predictor* pd_predictor = new PD_Predictor();
  paddle_infer::Config* config =
      reinterpret_cast<paddle_infer::Config*>(pd_config);
  pd_predictor->predictor = paddle_infer::CreatePredictor(*config);
  return pd_predictor;
}

__pd_give PD_Predictor* PD_PredictorClone(
    __pd_keep PD_Predictor* pd_predictor) {
  CHECK_AND_CONVERT_PD_PREDICTOR;
  PD_Predictor* new_predictor = new PD_Predictor();
  new_predictor->predictor = predictor->Clone();
  return new_predictor;
}

__pd_give PD_OneDimArrayCstr* PD_PredictorGetInputNames(
    __pd_keep PD_Predictor* pd_predictor) {
  CHECK_AND_CONVERT_PD_PREDICTOR;
  std::vector<std::string> names = predictor->GetInputNames();
  return paddle_infer::CvtVecToOneDimArrayCstr(names);
}

__pd_give PD_IOInfos* PD_PredictorGetInputInfos(
    __pd_keep PD_Predictor* pd_predictor) {
  CHECK_AND_CONVERT_PD_PREDICTOR;
  std::vector<std::string> names = predictor->GetInputNames();
  std::map<std::string, std::vector<int64_t>> input_shapes =
      predictor->GetInputTensorShape();
  std::map<std::string, paddle_infer::DataType> input_dtypes =
      predictor->GetInputTypes();

  PD_IOInfos* input_infos = new PD_IOInfos;
  input_infos->size = names.size();
  input_infos->io_info = names.empty() ? nullptr : new PD_IOInfo*[names.size()];
  for (size_t i = 0; i < names.size(); i++) {
    const std::string& name = names[i];
    input_infos->io_info[i] = new PD_IOInfo;
    input_infos->io_info[i]->name = paddle_infer::CvtStrToCstr(name);
    input_infos->io_info[i]->shape =
        paddle_infer::CvtVecToOneDimArrayInt64(input_shapes[name]);
    input_infos->io_info[i]->dtype =
        paddle_infer::CvtFromCxxDatatype(input_dtypes[name]);
  }
  return input_infos;
}

__pd_give PD_OneDimArrayCstr* PD_PredictorGetOutputNames(
    __pd_keep PD_Predictor* pd_predictor) {
  CHECK_AND_CONVERT_PD_PREDICTOR;
  std::vector<std::string> names = predictor->GetOutputNames();
  return paddle_infer::CvtVecToOneDimArrayCstr(names);
}

__pd_give PD_IOInfos* PD_PredictorGetOutputInfos(
    __pd_keep PD_Predictor* pd_predictor) {
  CHECK_AND_CONVERT_PD_PREDICTOR;
  std::vector<std::string> names = predictor->GetOutputNames();
  std::map<std::string, std::vector<int64_t>> output_shapes =
      predictor->GetOutputTensorShape();
  std::map<std::string, paddle_infer::DataType> output_dtypes =
      predictor->GetOutputTypes();

  PD_IOInfos* output_infos = new PD_IOInfos;
  output_infos->size = names.size();
  output_infos->io_info =
      names.empty() ? nullptr : new PD_IOInfo*[names.size()];
  for (size_t i = 0; i < names.size(); i++) {
    const std::string& name = names[i];
    output_infos->io_info[i] = new PD_IOInfo;
    output_infos->io_info[i]->name = paddle_infer::CvtStrToCstr(name);
    output_infos->io_info[i]->shape =
        paddle_infer::CvtVecToOneDimArrayInt64(output_shapes[name]);
    output_infos->io_info[i]->dtype =
        paddle_infer::CvtFromCxxDatatype(output_dtypes[name]);
  }
  return output_infos;
}

size_t PD_PredictorGetInputNum(__pd_keep PD_Predictor* pd_predictor) {
  CHECK_AND_CONVERT_PD_PREDICTOR;
  return predictor->GetInputNames().size();
}

size_t PD_PredictorGetOutputNum(__pd_keep PD_Predictor* pd_predictor) {
  CHECK_AND_CONVERT_PD_PREDICTOR;
  return predictor->GetOutputNames().size();
}
__pd_give PD_Tensor* PD_PredictorGetInputHandle(
    __pd_keep PD_Predictor* pd_predictor, const char* name) {
  CHECK_AND_CONVERT_PD_PREDICTOR;
  PD_Tensor* pd_tensor = new PD_Tensor();
  pd_tensor->tensor = predictor->GetInputHandle(name);
  return pd_tensor;
}

__pd_give PD_Tensor* PD_PredictorGetOutputHandle(
    __pd_keep PD_Predictor* pd_predictor, const char* name) {
  CHECK_AND_CONVERT_PD_PREDICTOR;
  PD_Tensor* pd_tensor = new PD_Tensor();
  pd_tensor->tensor = predictor->GetOutputHandle(name);
  return pd_tensor;
}

PD_Bool PD_PredictorRun(__pd_keep PD_Predictor* pd_predictor) {
  CHECK_AND_CONVERT_PD_PREDICTOR;
  return predictor->Run();  // NOLINT
}

void PD_PredictorClearIntermediateTensor(__pd_keep PD_Predictor* pd_predictor) {
  CHECK_AND_CONVERT_PD_PREDICTOR;
  predictor->ClearIntermediateTensor();
}

uint64_t PD_PredictorTryShrinkMemory(__pd_keep PD_Predictor* pd_predictor) {
  CHECK_AND_CONVERT_PD_PREDICTOR;
  return predictor->TryShrinkMemory();
}

void PD_PredictorDestroy(__pd_take PD_Predictor* pd_predictor) {
  delete pd_predictor;
}

const char* PD_GetVersion() {
  static std::string version = paddle_infer::GetVersion();
  return version.c_str();
}

}  // extern "C"
