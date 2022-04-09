//
// Created by disc on 1/17/2021.
//

#pragma comment(lib, "mfplat")

#include <cstdint>
#include <mfapi.h>
#include <mferror.h>
#include <mfidl.h>

#include "color_converter.h"
#include "utils.h"

color_converter::color_converter(int width, int height)
    : width_(width), height_(height) {
    MFT_REGISTER_TYPE_INFO in_filter;
    MFT_REGISTER_TYPE_INFO out_filter;

    in_filter.guidMajorType = MFMediaType_Video;
    in_filter.guidSubtype = MFVideoFormat_YUY2;

    out_filter.guidMajorType = MFMediaType_Video;
    out_filter.guidSubtype = MFVideoFormat_RGB32;

    CLSID *mft_ids;
    uint32_t mft_count = 0;
    THROW_IF_FAILED(MFTEnum(MFT_CATEGORY_VIDEO_PROCESSOR, 0, &in_filter,
                            &out_filter, nullptr, &mft_ids, &mft_count));

    if (mft_count == 0) { throw std::runtime_error("missing color converter"); }

    THROW_IF_FAILED(CoCreateInstance(mft_ids[0], nullptr, CLSCTX_INPROC_SERVER,
                                     IID_PPV_ARGS(&decoder_)));
    CoTaskMemFree(mft_ids);

    ComPtr<IMFMediaType> input_type;

    THROW_IF_FAILED(MFCreateMediaType(&input_type));
    input_type->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
    input_type->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_YUY2);
    MFSetAttributeSize(input_type.Get(), MF_MT_FRAME_SIZE, width_, height_);
    decoder_->SetInputType(0, input_type.Get(), 0);

    ComPtr<IMFMediaType> output_type;

    THROW_IF_FAILED(MFCreateMediaType(&output_type));
    output_type->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
    output_type->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_RGB32);
    MFSetAttributeSize(output_type.Get(), MF_MT_FRAME_SIZE, width_, height_);
    decoder_->SetOutputType(0, output_type.Get(), 0);

    ComPtr<IMFMediaBuffer> out_buffer;

    THROW_IF_FAILED(MFCreateMemoryBuffer(width_ * height_ * 4, &out_buffer));
    THROW_IF_FAILED(MFCreateSample(&out_sample_));
    THROW_IF_FAILED(out_sample_->AddBuffer(out_buffer.Get()));
}

ComPtr<IMFSample> color_converter::process(ComPtr<IMFSample> input) {
    decoder_->ProcessInput(0, input.Get(), 0);

    const DWORD out_buffer_count = 1;
    DWORD status = 0;
    MFT_OUTPUT_DATA_BUFFER out_buffer_info;

    out_buffer_info.pSample = out_sample_.Get();
    out_buffer_info.dwStreamID = 0;
    out_buffer_info.pEvents = nullptr;
    out_buffer_info.dwStatus = 0;

    HRESULT hr = decoder_->ProcessOutput(0, out_buffer_count, &out_buffer_info,
                                         &status);

    if (hr == S_OK) { return out_sample_; }

    if (hr != MF_E_TRANSFORM_NEED_MORE_INPUT) { THROW_IF_FAILED(hr); }

    return nullptr;
}
