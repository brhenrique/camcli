//
// Created by disc on 1/29/2021.
//

#ifndef CAM_CLI_STREAM_PIPELINE_H
#define CAM_CLI_STREAM_PIPELINE_H

#include "sample_filter.h"
#include "sample_sink.h"
#include "sample_stream.h"

#include <memory>
#include <thread>
#include <vector>

/**
 * Renders a camera stream into a window, converting the
 * input stream to RGB.
 */
class stream_pipeline {
public:
    stream_pipeline(std::unique_ptr<sample_stream> source,
                    std::vector<std::unique_ptr<sample_filter>> filters,
                    std::unique_ptr<sample_sink> sink);


    ~stream_pipeline();

    void start();

    void stop();

private:
    std::unique_ptr<sample_stream> source_;
    std::vector<std::unique_ptr<sample_filter>> filters_;
    std::unique_ptr<sample_sink> sink_;

    std::thread worker_;
};


#endif//CAM_CLI_STREAM_PIPELINE_H
