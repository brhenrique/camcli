//
// Created by disc on 1/29/2021.
//

#include "stream_pipeline.h"

stream_pipeline::stream_pipeline(
        std::unique_ptr<sample_stream> source,
        std::vector<std::unique_ptr<sample_filter>> filters,
        std::unique_ptr<sample_sink> sink)
    : source_(std::move(source)), filters_(std::move(filters)),
      sink_(std::move(sink)) {}

stream_pipeline::~stream_pipeline() {
    if (worker_.joinable()) { worker_.join(); }
}

void stream_pipeline::start() {
    source_->start(
            [&filters = filters_, &sink = sink_](ComPtr<IMFSample> sample) {
                for (const auto &filter : filters) {
                    if (!sample) { break; }

                    sample = filter->process(sample);
                }

                if (!sample) { return; }

                sink->write(sample);
            });
}

void stream_pipeline::stop() { source_->stop(); }
