#pragma once

#ifdef FMT_HEADER_ONLY
#undef FMT_HEADER_ONLY
#endif
#ifdef SPDLOG_FMT_EXTERNAL
#undef SPDLOG_FMT_EXTERNAL
#endif
#include <spl/logger/Logger.h>
#include <spl/pipeline/SplPipeline.h>
#include <stdint.h>

#include "models/PipelineEventGroup.h"
#include "pipeline/PipelineContext.h"

namespace logtail {

enum class SPLStatusCode {
    OK, // create/execute
    TIMEOUT_ERROR, // execute
    USER_ERROR, // create/execute
    FATAL_ERROR, // create/execute
    MEM_EXCEEDED, // create/execute
};

struct Error {
    SPLStatusCode code_;
    std::string msg_;
    Error() { set(SPLStatusCode::OK, ""); }
    Error(const SPLStatusCode& statusCode, const std::string& message) { set(statusCode, message); }

    // check is Error is ok
    bool ok() { return code_ == SPLStatusCode::OK; }
    void reset() { set(SPLStatusCode::OK, ""); }
    void set(const SPLStatusCode& statusCode, const std::string& message) {
        code_ = statusCode;
        msg_ = message;
    }
};

struct TaskStats {
    uint64_t inputBytes_{0};
    uint64_t outputBytes_{0};
    int64_t durationMicros_{0};
    // mem
    int64_t memPeakBytes_;
    int32_t inputRows_{0};
    int32_t outputRows_{0};
};

struct PipelineStats {
    // task
    int64_t processMicros_{0};
    int64_t inputMicros_{0};
    int64_t outputMicros_{0};
    int64_t memPeakBytes_{0};
    int32_t totalTaskCount_{0};
    int32_t succTaskCount_{0};
    int32_t failTaskCount_{0};
    std::vector<TaskStats> taskStats_;
    PipelineStats() = default;
    PipelineStats(const PipelineStats& ps);
};


class LogtailSplPipeline {
public:
    LogtailSplPipeline() = default;
    ~LogtailSplPipeline() = default;

    Error InitLogtailSPL(std::string spl,
                         const int32_t pipelineQuota,
                         const int32_t queryMaxSize,
                         uint32_t const timeoutMills = 100,
                         const int64_t maxMemoryBytes = 1024 * 1024 * 1024,
                         Logger::logger logger = nullptr);
    Error Execute(PipelineEventGroup logGroup,
                  std::vector<PipelineEventGroup>& logGroupList,
                  PipelineStats& pipelineStats,
                  PipelineContext* context);

private:
    std::shared_ptr<apsara::sls::spl::SplPipeline> mSPLPipelinePtr;
};
} // namespace logtail