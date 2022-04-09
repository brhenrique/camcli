//
// Created by disc on 1/16/2021.
//

#ifndef CAM_CLI_MEDIA_SOURCE_STREAM_H
#define CAM_CLI_MEDIA_SOURCE_STREAM_H

#include <atomic>
#include <memory>
#include <thread>

#include "sample_stream.h"

class IMFSourceReader;

/**
 * Stream of IMFSamples from a IMFMediaSource
 */
class media_source_stream : public sample_stream {
public:
    /**
     * |source| will be moved to the instance
     */
    media_source_stream(ComPtr<IMFMediaSource> &&source, properties props);

    ~media_source_stream();

    /**
     * Creates a separate thread for capture, |on_data_cb| will be called in the
     * capture thread.
     */
    void start(OnDataCallback on_data_cb) override;

    void stop() override;

    properties get_properties() const override;

private:
    void run_capture();

    void create_reader();

    void find_resolution();

    ComPtr<IMFSourceReader> mf_reader_;
    ComPtr<IMFMediaSource> source_;

    std::atomic<bool> active_;
    std::thread capture_thread_;
    OnDataCallback on_data_cb_;
    properties target_props_;
};

#endif//CAM_CLI_MEDIA_SOURCE_STREAM_H
