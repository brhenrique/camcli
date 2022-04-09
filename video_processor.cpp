//
// Created by disc on 2/14/2021.
//

#include <mfapi.h>
#include <mferror.h>
#include <mfidl.h>

#include "utils.h"
#include "video_processor.h"
#include "win_graphics.h"

#include <d3d11.h>
#include <fstream>

// move to factory
video_processor::video_processor(int width, int height, uintptr_t d3d_manager)
    : width_(width), height_(height) {
    std::cout << "d3d manager: " << d3d_manager << std::endl;

    THROW_IF_FAILED(CoCreateInstance(CLSID_VideoProcessorMFT, nullptr,
                                     CLSCTX_INPROC_SERVER,
                                     IID_PPV_ARGS(&decoder_)));
    ComPtr<IMFMediaType> input_type;
    THROW_IF_FAILED(MFCreateMediaType(&input_type));
    input_type->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
    input_type->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_YUY2);
    input_type->SetUINT32(MF_MT_ALL_SAMPLES_INDEPENDENT, TRUE);
    MFSetAttributeSize(input_type.Get(), MF_MT_FRAME_SIZE, width_, height_);
    THROW_IF_FAILED(decoder_->SetInputType(0, input_type.Get(), 0));

    ComPtr<IMFMediaType> output_type;
    THROW_IF_FAILED(MFCreateMediaType(&output_type));
    output_type->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
    output_type->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_ARGB32);
    output_type->SetUINT32(MF_MT_ALL_SAMPLES_INDEPENDENT, TRUE);
    output_type->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive);
    MFSetAttributeSize(output_type.Get(), MF_MT_FRAME_SIZE, width_, height_);
    THROW_IF_FAILED(decoder_->SetOutputType(0, output_type.Get(), 0));

    ComPtr<IMFAttributes> mft_attr;
    THROW_IF_FAILED(decoder_->GetAttributes(&mft_attr));

    uint32_t is_async = 0;
    mft_attr->GetUINT32(MF_TRANSFORM_ASYNC, &is_async);

    if (is_async) {
        // Unlock asynchronous mode
        std::cout << "is async" << std::endl;
        throw std::runtime_error("asynchronous MFTs are not supported");
    }

    // Connect D3D device
    THROW_IF_FAILED(
            decoder_->ProcessMessage(MFT_MESSAGE_SET_D3D_MANAGER, d3d_manager));

    CreateSurface(d3d_manager);
}
void video_processor::CreateSurface(uintptr_t d3d_manager) {
    dxgi_manager_ = Microsoft::WRL::ComPtr<IMFDXGIDeviceManager>(
            reinterpret_cast<IMFDXGIDeviceManager *>(d3d_manager));

    Microsoft::WRL::ComPtr<ID3D11Device> d3d_device;
    HANDLE device_handle = nullptr;
    THROW_IF_FAILED(dxgi_manager_->OpenDeviceHandle(&device_handle));
    THROW_IF_FAILED(dxgi_manager_->GetVideoService(
            device_handle, IID_PPV_ARGS(d3d_device.GetAddressOf())));

    Microsoft::WRL::ComPtr<IDXGIDevice> dxgi_device;
    THROW_IF_FAILED(d3d_device.As(&dxgi_device));

    DXGI_SURFACE_DESC surface_desc{width_, height_, DXGI_FORMAT_YUY2, {1, 0}};

    THROW_IF_FAILED(dxgi_device->CreateSurface(
            &surface_desc, 1, DXGI_USAGE_UNORDERED_ACCESS, nullptr, &surface_));
}

ComPtr<IMFSample> video_processor::process(ComPtr<IMFSample> input) {
    const DWORD out_buffer_count = 1;
    DWORD status = 0;
    MFT_OUTPUT_DATA_BUFFER out_buffer_info;

    MFT_OUTPUT_STREAM_INFO stream_info;
    THROW_IF_FAILED(decoder_->GetOutputStreamInfo(0, &stream_info));

    if (!(stream_info.dwFlags & MFT_OUTPUT_STREAM_PROVIDES_SAMPLES)) {
        throw std::runtime_error("video_processor does not support MFT that "
                                 "doesnt provide samples");
    }

    out_buffer_info.pSample = nullptr;
    out_buffer_info.dwStreamID = 0;
    out_buffer_info.pEvents = nullptr;
    out_buffer_info.dwStatus = 0;

    HRESULT hr = decoder_->ProcessOutput(0, out_buffer_count, &out_buffer_info,
                                         &status);

    if (hr == S_OK) {
        ComPtr<IMFSample> result;
        result.Attach(out_buffer_info.pSample);

        return result;
    }

    if (hr != MF_E_TRANSFORM_NEED_MORE_INPUT) { THROW_IF_FAILED(hr); }

    hr = decoder_->ProcessInput(0, input.Get(), 0);

    if (hr == E_NOINTERFACE) {
        THROW_IF_FAILED(
                decoder_->ProcessMessage(MFT_MESSAGE_COMMAND_FLUSH, NULL));

        input = SampleToDxgiSurface(surface_, input);

        hr = decoder_->ProcessInput(0, input.Get(), 0);
    }

    if (hr == S_OK || hr == MF_E_NOTACCEPTING) { return {}; }

    THROW_IF_FAILED(hr);
}
