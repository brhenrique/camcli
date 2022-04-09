//
// Created by disc on 2/14/2021.
//

#ifndef CAM_CLI_VIDEO_PROCESSOR_H
#define CAM_CLI_VIDEO_PROCESSOR_H

#include <dxgi.h>

#include <cstdint>

#include "sample_filter.h"

class IMFTransform;

class video_processor : public sample_filter {
public:
    video_processor(int width, int height, uintptr_t d3d_manager);

    ComPtr<IMFSample> process(ComPtr<IMFSample> input) override;

private:
    ComPtr<IMFTransform> decoder_;
    ComPtr<IMFSample> out_sample_;

    // Used to move input from system memory to GPU
    ComPtr<IMFSample> in_sample_;
    ComPtr<IDXGISurface> surface_;
    ComPtr<IMFDXGIDeviceManager> dxgi_manager_;

    uint32_t width_;
    uint32_t height_;
    void CreateSurface(uintptr_t d3d_manager);
};


#endif//CAM_CLI_VIDEO_PROCESSOR_H
