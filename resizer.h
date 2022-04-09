//
// Created by disc on 3/7/2021.
//

#ifndef CAM_CLI_RESIZER_H
#define CAM_CLI_RESIZER_H

#include "sample_filter.h"

class ID2D1Effect;
class ID2D1DeviceContext;
class ID2D1Bitmap;

class resizer : public sample_filter {
public:
    resizer(size_t in_width, size_t in_height, float scale_factor);

    ComPtr<IMFSample> process(ComPtr<IMFSample> input) override;

private:
    ComPtr<ID2D1Effect> effect_;
    ComPtr<ID2D1DeviceContext> dc_;
    ComPtr<ID2D1Bitmap> bitmap_;

    size_t in_width_;
    size_t in_height_;
    float scale_factor_;
};

#endif//CAM_CLI_RESIZER_H
