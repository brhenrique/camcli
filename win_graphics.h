//
// Created by disc on 3/6/2021.
//

#ifndef CAM_CLI_WIN_GRAPHICS_H
#define CAM_CLI_WIN_GRAPHICS_H

#include <d2d1_2.h>
#include <d3d11.h>
#include <mfapi.h>
#include <wrl/client.h>

#include <tuple>

#include "utils.h"

using Microsoft::WRL::ComPtr;

ComPtr<IMFSample> SampleToDxgiSurface(const ComPtr<IDXGISurface> &surface,
                                      const ComPtr<IMFSample> &input);

void SampleToBitmap(const ComPtr<IMFSample> &input,
                    const ComPtr<ID2D1Bitmap> &output, uint32_t width,
                    uint32_t height, uint32_t stride);

ComPtr<ID2D1Bitmap> GpuSampleToBitmap(ComPtr<IMFSample> sample,
                                      ComPtr<ID2D1DeviceContext> dc);

#endif//CAM_CLI_WIN_GRAPHICS_H
