//
// Created by disc on 3/6/2021.
//

#ifndef CAM_CLI_SAMPLE_STREAM_H
#define CAM_CLI_SAMPLE_STREAM_H

#include <mfobjects.h>
#include <wrl/client.h>

#include <cstdint>

#include <functional>

using Microsoft::WRL::ComPtr;

/**
 * Defines a stream of IMFSample, where all samples have the same |properties|
 */
class sample_stream {
public:
    typedef std::function<void(ComPtr<IMFSample>)> OnDataCallback;

    struct properties {
        uint32_t width;
        uint32_t height;
        enum class pixel_format { yuy2, unknown } pixel_format;
    };

    virtual void start(OnDataCallback on_data_cb) = 0;

    virtual void stop() = 0;

    [[nodiscard]] virtual properties get_properties() const = 0;
};

#endif//CAM_CLI_SAMPLE_STREAM_H
