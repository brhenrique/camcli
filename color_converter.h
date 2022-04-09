//
// Created by disc on 1/17/2021.
//

#ifndef CAM_CLI_COLOR_CONVERTER_H
#define CAM_CLI_COLOR_CONVERTER_H

#include <vector>

#include "sample_filter.h"

class IMFTransform;

class color_converter : public sample_filter {
public:
    color_converter(int width, int height);

    ComPtr<IMFSample> process(ComPtr<IMFSample> input) override;

private:
    ComPtr<IMFTransform> decoder_;
    ComPtr<IMFSample> out_sample_;
    uint32_t width_;
    uint32_t height_;
};


#endif//CAM_CLI_COLOR_CONVERTER_H
