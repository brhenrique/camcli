//
// Created by disc on 3/14/2021.
//

#include "win_graphics.h"

ComPtr<IMFSample> SampleToDxgiSurface(const ComPtr<IDXGISurface> &surface,
                                      const ComPtr<IMFSample> &input) {
    ComPtr<IMFSample> output;
    MFCreateSample(&output);

    ComPtr<IMFMediaBuffer> input_buffer;
    THROW_IF_FAILED(input->ConvertToContiguousBuffer(&input_buffer));

    ComPtr<IMFMediaBuffer> buffer;
    THROW_IF_FAILED(MFCreateDXGISurfaceBuffer(
            __uuidof(ID3D11Texture2D), surface.Get(), 0, FALSE, &buffer));

    uint8_t *input_data = nullptr;
    uint8_t *output_data = nullptr;
    DWORD src_size = 0;
    DWORD dest_size = 0;

    THROW_IF_FAILED(input_buffer->Lock(&input_data, nullptr, &src_size));
    THROW_IF_FAILED(buffer->Lock(&output_data, &dest_size, nullptr));
    memcpy_s(output_data, dest_size, input_data, src_size);
    THROW_IF_FAILED(buffer->Unlock());
    THROW_IF_FAILED(input_buffer->Unlock());
    THROW_IF_FAILED(buffer->SetCurrentLength(src_size));

    int64_t duration = 0;
    int64_t pts = 0;
    THROW_IF_FAILED(input->GetSampleDuration(&duration));
    THROW_IF_FAILED(input->GetSampleTime(&pts));

    THROW_IF_FAILED(output->SetSampleDuration(duration));
    THROW_IF_FAILED(output->SetSampleTime(pts));

    THROW_IF_FAILED(output->AddBuffer(buffer.Get()));

    return output;
}

void SampleToBitmap(const ComPtr<IMFSample> &input,
                    const ComPtr<ID2D1Bitmap> &output, uint32_t width,
                    uint32_t height, uint32_t stride) {
    ComPtr<IMFMediaBuffer> in_buffer;
    THROW_IF_FAILED(input->ConvertToContiguousBuffer(&in_buffer));

    uint8_t *data = nullptr;
    DWORD data_size = 0;
    THROW_IF_FAILED(in_buffer->Lock(&data, nullptr, &data_size));

    const D2D_RECT_U src_area{0, 0, width, height};

    THROW_IF_FAILED(output->CopyFromMemory(&src_area, data, stride));
    THROW_IF_FAILED(in_buffer->Unlock());
}

ComPtr<ID2D1Bitmap> GpuSampleToBitmap(ComPtr<IMFSample> sample,
                                      ComPtr<ID2D1DeviceContext> dc) {
    ComPtr<IMFMediaBuffer> src_buffer;
    RETURN_IF_FAILED(sample->GetBufferByIndex(0, &src_buffer));

    DWORD buffer_size = 0;
    src_buffer->GetCurrentLength(&buffer_size);

    ComPtr<IMFDXGIBuffer> src_dxgi_buffer;
    RETURN_IF_FAILED(src_buffer.As(&src_dxgi_buffer));

    ComPtr<IDXGISurface1> src_surface;
    RETURN_IF_FAILED(src_dxgi_buffer->GetResource(IID_PPV_ARGS(&src_surface)));

    D2D1_BITMAP_PROPERTIES properties = {};
    properties.pixelFormat = {DXGI_FORMAT_B8G8R8A8_UNORM,
                              D2D1_ALPHA_MODE_PREMULTIPLIED};
    properties.dpiX = 0;
    properties.dpiY = 0;

    ComPtr<ID2D1Bitmap> bitmap;
    RETURN_IF_FAILED(dc->CreateSharedBitmap(
            __uuidof(IDXGISurface), src_surface.Get(), &properties, &bitmap));

    return bitmap;
}
