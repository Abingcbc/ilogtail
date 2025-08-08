/*
 * Copyright 2024 iLogtail Authors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <string>

#include "host_monitor/HostMonitorTimerEvent.h"
#include "models/PipelineEventGroup.h"

namespace logtail {

class BaseCollector {
public:
    virtual ~BaseCollector() = default;

    virtual bool Init(HostMonitorTimerEvent::CollectContext& collectContext) {
        auto collectInterval = GetCollectInterval();
        if (collectInterval.count() == 0) {
            // Collector not support statistics metrics, set to 1
            mCountPerReport = 1;
            mCount = 0;
            collectContext.mCollectInterval = collectContext.mReportInterval;
            return true;
        }
        mCountPerReport = collectContext.mReportInterval.count() / collectInterval.count();
        mCount = 0;
        collectContext.mCollectInterval = collectInterval;
        return true;
    }
    virtual bool Collect(const HostMonitorTimerEvent::CollectContext& collectContext, PipelineEventGroup* group) = 0;
    [[nodiscard]] virtual const std::string& Name() const = 0;
    [[nodiscard]] virtual const std::chrono::seconds GetCollectInterval() const = 0;

protected:
    bool mValidState = true;

    // basic multi-value metrics
    int mCountPerReport = 0;
    int mCount = 0;
};

} // namespace logtail
