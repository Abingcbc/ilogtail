// Copyright 2023 iLogtail Authors
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

#include "collection_pipeline/plugin/instance/InputInstance.h"

namespace logtail {
bool InputInstance::Init(const Json::Value& config,
                         CollectionPipelineContext& context,
                         size_t inputIdx,
                         Json::Value& optionalGoPipeline) {
    mPlugin->SetContext(context);
    mPlugin->CreateMetricsRecordRef(Name(), PluginID());
    mPlugin->SetInputIndex(inputIdx);
    if (!mPlugin->Init(config, optionalGoPipeline)) {
        return false;
    }
    mPlugin->CommitMetricsRecordRef();
    return true;
}

} // namespace logtail
