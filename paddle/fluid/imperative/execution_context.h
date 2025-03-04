// Copyright (c) 2020 PaddlePaddle Authors. All Rights Reserved.
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

#pragma once

#include <string>
#include <vector>

#include "paddle/fluid/framework/operator.h"
#include "paddle/fluid/framework/type_defs.h"
#include "paddle/fluid/framework/variable.h"
#include "paddle/fluid/imperative/type_defs.h"
#include "paddle/fluid/imperative/var_helper.h"

namespace paddle {
namespace imperative {

template <typename VarType>
class DygraphExecutionContext : public framework::ExecutionContext {
  using Variable = framework::Variable;

 public:
  DygraphExecutionContext(const framework::OperatorBase& op,
                          const framework::Scope& scope,
                          const phi::DeviceContext& device_context,
                          const framework::RuntimeContext& ctx,
                          const NameVarMap<VarType>& var_map_in,
                          const NameVarMap<VarType>& var_map_out,
                          const framework::AttributeMap& attrs,
                          const framework::AttributeMap& default_attrs)
      : ExecutionContext(op, scope, device_context, ctx),
        var_map_in_(var_map_in),
        var_map_out_(var_map_out),
        attrs_(attrs),
        default_attrs_(default_attrs) {}

  std::string InputName(const std::string& name) const override {
    auto it = var_map_in_.find(name);
    PADDLE_ENFORCE_NE(it,
                      var_map_in_.end(),
                      platform::errors::PreconditionNotMet(
                          "Can not find [%s] in Input", name));
    return it->second[0] ? GetNameFromVar(it->second[0])
                         : framework::kEmptyVarName;
  }

  std::vector<std::string> InputNames(const std::string& name) const override {
    auto it = var_map_in_.find(name);
    PADDLE_ENFORCE_NE(
        it,
        var_map_in_.end(),
        platform::errors::NotFound("Can not find [%s] in Input", name));
    std::vector<std::string> vec_res;
    vec_res.reserve(it->second.size());
    for (size_t i = 0; i < it->second.size(); ++i) {
      if (it->second[i]) {
        vec_res.push_back(GetNameFromVar(it->second[i]));
      } else {
        vec_res.push_back(framework::kEmptyVarName);
      }
    }
    return vec_res;
  }

  std::string OutputName(const std::string& name) const override {
    auto it = var_map_out_.find(name);
    PADDLE_ENFORCE_NE(
        it,
        var_map_out_.end(),
        platform::errors::NotFound("Can not find [%s] in Output", name));
    return it->second[0] ? GetNameFromVar(it->second[0])
                         : framework::kEmptyVarName;
  }

  std::vector<std::string> OutputNames(const std::string& name) const override {
    auto it = var_map_out_.find(name);
    PADDLE_ENFORCE_NE(
        it,
        var_map_out_.end(),
        platform::errors::NotFound("Can not find [%s] in Output", name));
    std::vector<std::string> vec_res;
    vec_res.reserve(it->second.size());
    for (size_t i = 0; i < it->second.size(); ++i) {
      if (it->second[i]) {
        vec_res.push_back(GetNameFromVar(it->second[i]));
      } else {
        vec_res.push_back(framework::kEmptyVarName);
      }
    }
    return vec_res;
  }

  bool HasAttr(const std::string& name) const override {
    return attrs_.find(name) != attrs_.end() ||
           default_attrs_.find(name) != default_attrs_.end();
  }

  const framework::AttributeMap& Attrs() const override { return attrs_; }

  const framework::Attribute& GetAttr(const std::string& name) const override {
    auto it = attrs_.find(name);

    if (it == attrs_.end()) {
      it = default_attrs_.find(name);
      if (it == default_attrs_.end()) {
        PADDLE_THROW(platform::errors::NotFound(
            "Can not find [%s] in attributes of op %s.",
            name,
            this->GetOp().Type()));
      }
    }

    return it->second;
  }

  paddle::small_vector<const std::string*> InNameList() const override {
    paddle::small_vector<const std::string*> vec_temp;
    vec_temp.reserve(var_map_in_.size());

    for (auto& v : var_map_in_) {
      vec_temp.push_back(&v.first);
    }

    return vec_temp;
  }

  bool HasInput(const std::string& name) const override {
    auto it = var_map_in_.find(name);
    return (it != var_map_in_.end() && it->second.size() > 0);
  }

  bool HasInputs(const std::string& name) const override {
    auto it = var_map_in_.find(name);
    return (it != var_map_in_.end() && it->second.size() > 0);
  }

  bool HasOutput(const std::string& name) const override {
    auto it = var_map_out_.find(name);
    return (it != var_map_out_.end() && it->second.size() > 0);
  }

  size_t InputSize(const std::string& name) const override {
    auto it = var_map_in_.find(name);
    PADDLE_ENFORCE_NE(
        it,
        var_map_in_.end(),
        platform::errors::NotFound("Can not find [%s] in Input", name));
    return it->second.size();
  }

  size_t OutputSize(const std::string& name) const override {
    auto it = var_map_out_.find(name);
    PADDLE_ENFORCE_NE(
        it,
        var_map_out_.end(),
        platform::errors::NotFound("Can not find [%s] in Output", name));
    return it->second.size();
  }

  const Variable* InputVar(const std::string& name) const override {
    auto it = var_map_in_.find(name);
    if (it == var_map_in_.end()) {
      return nullptr;
    }

    return it->second.empty() || it->second[0] == nullptr
               ? nullptr
               : it->second[0]->MutableVar();
  }

  Variable* OutputVar(const std::string& name) const override {
    auto it = var_map_out_.find(name);
    if (it == var_map_out_.end()) {
      return nullptr;
    }

    return it->second.empty() || it->second[0] == nullptr
               ? nullptr
               : it->second[0]->MutableVar();
  }

  const std::vector<Variable*> MultiInputVar(
      const std::string& name) const override {
    auto it = var_map_in_.find(name);
    if (it == var_map_in_.end()) {
      return {};
    }
    std::vector<Variable*> vec_res;
    vec_res.reserve(it->second.size());
    for (size_t i = 0; i < it->second.size(); ++i) {
      vec_res.push_back(it->second[i] ? it->second[i]->MutableVar() : nullptr);
    }

    return vec_res;
  }

  std::vector<Variable*> MultiOutputVar(
      const std::string& name) const override {
    auto it = var_map_out_.find(name);
    if (it == var_map_out_.end()) {
      return {};
    }
    std::vector<Variable*> vec_res;
    vec_res.reserve(it->second.size());
    for (size_t i = 0; i < it->second.size(); ++i) {
      vec_res.push_back(it->second[i] ? it->second[i]->MutableVar() : nullptr);
    }

    return vec_res;
  }

 private:
  const NameVarMap<VarType>& var_map_in_;
  const NameVarMap<VarType>& var_map_out_;
  const framework::AttributeMap& attrs_;
  const framework::AttributeMap& default_attrs_;
};

}  // namespace imperative
}  // namespace paddle
