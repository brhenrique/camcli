//
// Created by disc on 1/16/2021.
//

#pragma comment(lib, "mfreadwrite")
#pragma comment(lib, "mfplat")
#pragma comment(lib, "mf")
#pragma comment(lib, "mfuuid")

#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>


#include <utility>

#include "media_source_stream.h"
#include "utils.h"

media_source_stream::media_source_stream(ComPtr<IMFMediaSource> &&source,
                                         properties props)
    : source_(std::move(source)), target_props_(props) {
    create_reader();
}

void media_source_stream::start(OnDataCallback on_data_cb) {
    on_data_cb_ = std::move(on_data_cb);
    active_ = true;

    capture_thread_ = std::thread([this]() { run_capture(); });
}

void media_source_stream::create_reader() {
    ComPtr<IMFAttributes> attributes;

    THROW_IF_FAILED(MFCreateSourceReaderFromMediaSource(source_.Get(), nullptr,
                                                        &mf_reader_));
    find_resolution();

    // check if yuy2
    ComPtr<IMFMediaType> curr_type;
    mf_reader_->GetCurrentMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM,
                                    &curr_type);
    GUID pixel_format;
    curr_type->GetGUID(MF_MT_SUBTYPE, &pixel_format);

    std::cout << "is yuy2? " << (pixel_format == MFVideoFormat_YUY2)
              << std::endl;
}

void media_source_stream::run_capture() {
    ComPtr<IMFSample> sample;
    DWORD stream_flags = 0;

    while (active_) {
        THROW_IF_FAILED(mf_reader_->ReadSample(
                MF_SOURCE_READER_FIRST_VIDEO_STREAM, 0, nullptr, &stream_flags,
                nullptr, &sample));

        if (stream_flags & MF_SOURCE_READERF_STREAMTICK) { continue; }
        if (!on_data_cb_) { continue; }

        on_data_cb_(sample);
    }
}

void media_source_stream::stop() {
    active_ = false;

    if (capture_thread_.joinable()) { capture_thread_.join(); }
}

media_source_stream::properties media_source_stream::get_properties() const {
    ComPtr<IMFMediaType> media_type;
    THROW_IF_FAILED(mf_reader_->GetCurrentMediaType(
            MF_SOURCE_READER_FIRST_VIDEO_STREAM, &media_type));

    properties p{};
    THROW_IF_FAILED(MFGetAttributeSize(media_type.Get(), MF_MT_FRAME_SIZE,
                                       &p.width, &p.height));
    return p;
}

media_source_stream::~media_source_stream() {
    if (active_) { stop(); }
}

void media_source_stream::find_resolution() {
    const std::vector<std::pair<int, int>> candidates = {
            {3840, 2160}, {2560, 1440}, {1920, 1080}, {1600, 900},
            {1400, 900},  {1280, 720},  {854, 480}};

    ComPtr<IMFMediaType> media_type;
    THROW_IF_FAILED(mf_reader_->GetCurrentMediaType(
            MF_SOURCE_READER_FIRST_VIDEO_STREAM, &media_type));

    for (const auto &[width, height] : candidates) {
        if (width * height > target_props_.width * target_props_.height) {
            continue;
        }

        THROW_IF_FAILED(MFSetAttributeSize(media_type.Get(), MF_MT_FRAME_SIZE,
                                           width, height));

        if (mf_reader_->SetCurrentMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM,
                                            nullptr,
                                            media_type.Get()) == S_OK) {
            std::cout << "selected resolution: " << width << " x " << height
                      << std::endl;
            break;
        }
    }
}