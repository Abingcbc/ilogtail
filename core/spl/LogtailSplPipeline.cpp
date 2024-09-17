#include "LogtailSplPipeline.h"

#include <spl/json.hpp>

#include "spl/PipelineEventGroupInput.h"
#include "spl/PipelineEventGroupOutput.h"
#include "spl/SplConstants.h"

using namespace apsara::sls::spl;

namespace logtail {
Error LogtailSplPipeline::InitLogtailSPL(std::string spl,
                                         const int32_t pipelineQuota,
                                         const int32_t queryMaxSize,
                                         const uint32_t timeoutMills,
                                         const int64_t maxMemoryBytes,
                                         Logger::logger logger) {
    PipelineOptions splOptions;
    // different parse mode support different spl operators
    splOptions.parserMode = parser::ParserMode::LOGTAIL;
    // spl pipeline的长度，多少管道
    splOptions.pipelineQuota = pipelineQuota;
    // spl pipeline语句的最大长度
    splOptions.queryMaxSize = queryMaxSize;
    // sampling for error
    splOptions.errorSampling = true;

    // this function is void and has no return
    initSPL(&splOptions);

    apsara::sls::spl::Error error;
    mSPLPipelinePtr = std::make_shared<apsara::sls::spl::SplPipeline>(
        spl, error, (u_int64_t)timeoutMills, (int64_t)maxMemoryBytes, logger);
    return Error(static_cast<SPLStatusCode>(error.code_), error.msg_);
}

Error LogtailSplPipeline::Execute(PipelineEventGroup logGroup,
                                  std::vector<PipelineEventGroup>& logGroupList,
                                  PipelineStats& pipelineStats,
                                  PipelineContext* context) {
    std::string errorMsg;
    Error error = Error();
    std::vector<std::string> colNames{FIELD_CONTENT};
    // 根据spip->getInputSearches()，设置input数组
    std::vector<Input*> inputs;
    for (const auto& search : mSPLPipelinePtr->getInputSearches()) {
        (void)search; //-Wunused-variable
        PipelineEventGroupInput* input = new PipelineEventGroupInput(colNames, logGroup, *context);
        if (!input) {
            logGroupList.emplace_back(std::move(logGroup));
            for (auto& input : inputs) {
                delete input;
            }
            return error;
        }
        inputs.push_back(input);
    }
    // 根据spip->getOutputLabels()，设置output数组
    std::vector<Output*> outputs;
    for (const auto& resultTaskLabel : mSPLPipelinePtr->getOutputLabels()) {
        PipelineEventGroupOutput* output
            = new PipelineEventGroupOutput(logGroup, logGroupList, *context, resultTaskLabel);
        if (!output) {
            logGroupList.emplace_back(std::move(logGroup));
            for (auto& input : inputs) {
                delete input;
            }
            for (auto& output : outputs) {
                delete output;
            }
            return error;
        }
        outputs.emplace_back(output);
    }

    // 开始调用pipeline.execute
    // 传入inputs, outputs
    // 输出pipelineStats, error
    auto splPipelineStats = apsara::sls::spl::PipelineStats();
    auto errCode = static_cast<SPLStatusCode>(mSPLPipelinePtr->execute(inputs, outputs, &errorMsg, &splPipelineStats));
    pipelineStats.processMicros_ = splPipelineStats.processMicros_;
    pipelineStats.inputMicros_ = splPipelineStats.inputMicros_;
    pipelineStats.outputMicros_ = splPipelineStats.outputMicros_;
    pipelineStats.memPeakBytes_ = splPipelineStats.memPeakBytes_;
    pipelineStats.totalTaskCount_ = splPipelineStats.totalTaskCount_;
    pipelineStats.succTaskCount_ = splPipelineStats.succTaskCount_;
    pipelineStats.failTaskCount_ = splPipelineStats.failTaskCount_;

    for (auto& input : inputs) {
        delete input;
    }
    for (auto& output : outputs) {
        delete output;
    }
    error.set(errCode, errorMsg);

    if (error.code_ != SPLStatusCode::OK) {
        LOG_ERROR(
            sLogger,
            ("execute error", errorMsg)("project", context->GetProjectName())("logstore", context->GetLogstoreName())(
                "region", context->GetRegion())("configName", context->GetConfigName()));
        // 出现错误，把原数据放回来
        logGroupList.emplace_back(std::move(logGroup));
    }
    return error;
}
} // namespace logtail