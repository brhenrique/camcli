//
// Created by disc on 2/3/2021.
//

#include <atlbase.h>
#include <atlconv.h>

#include "bitmap_sink.h"
#include "camera.h"
#include "color_converter.h"
#include "d3d_factory.h"
#include "media_source_stream.h"
#include "mf_session.h"
#include "resizer.h"
#include "stream_pipeline.h"
#include "video_processor.h"
#include "window_renderer.h"

#include <iostream>

struct CliOptions {
    std::string camera_name;
    uint32_t source_width = 1280;
    uint32_t source_height = 720;
    uint32_t window_width = 854;
    uint32_t window_height = 480;
    std::string window_shape;
    bool enable_hw = true;
};

void display_cli_help() {
    // clang-format off
    std::cout << "CamCLI."
                     "\nUsage:"
                     "\n  cam_cli show <device_name> [--mirror --crop --size --shape]"
                     "\n  cam_cli list-devices"
                     "\n  cam_cli -h | --help"

                     "\nOptions: "
                     "\n  -h --help                                         Show this screen."
                     "\n  --source-size=<width> <height>                    Target resolution of the camera stream"
                     "\n  --window-size=<width> <height>                    Size of the window where the camera frames will be displayed"
                     "\n  --shape=rectangle|circle                          Window shape"
                     "\n  --mirror                                          Mirror rendered stream";
    // clang-format on
}

std::unique_ptr<bitmap_sink>
create_bitmap_sink(bitmap_sink::window_ui window_ui,
                   const ComPtr<ID3D11Device> &d3d_device) {

    if (d3d_device) {
        return std::make_unique<bitmap_sink>(window_ui, d3d_device);
    }

    return std::make_unique<bitmap_sink>(window_ui);
}

std::unique_ptr<sample_filter>
create_color_conversion_filter(uint32_t source_width, uint32_t source_height,
                               const ComPtr<ID3D11Device> &d3d_device) {
    if (d3d_device) {
        auto mf_manager = d3d_factory::CreateMfDeviceManager(d3d_device.Get());
        auto mf_manager_ptr = reinterpret_cast<intptr_t>(mf_manager.Get());

        return std::make_unique<video_processor>(source_width, source_height,
                                                 mf_manager_ptr);
    }

    return std::make_unique<color_converter>(source_width, source_height);
}

std::unique_ptr<sample_filter>
create_resizing_filter(uint32_t source_width, uint32_t source_height,
                       float scale_factor,
                       const ComPtr<ID3D11Device> d3d_device) {
    if (d3d_device) {
        auto mf_manager = d3d_factory::CreateMfDeviceManager(d3d_device.Get());
        auto mf_manager_ptr = reinterpret_cast<intptr_t>(mf_manager.Get());

        return std::make_unique<video_processor>(source_width, source_height,
                                                 mf_manager_ptr);
    }

    return std::make_unique<resizer>(source_width, source_height, scale_factor);
}

void run_pipeline(const CliOptions &options) {
    const auto &camera_name = options.camera_name;
    const auto window_width = options.window_width;
    const auto window_height = options.window_height;
    const auto enable_hw = options.enable_hw;
    auto source_width = options.source_width;
    auto source_height = options.source_height;

    mf_session session_handle;

    ATL::CA2W camera_name_wide(camera_name.c_str());
    // Create media source to capture the raw camera frames
    auto cam_source = std::make_unique<media_source_stream>(
            camera::create_media_source(std::wstring(camera_name_wide)),
            media_source_stream::properties{source_width, source_height});

    // Values may differ from the ones asked for if the source does not support them
    source_width = cam_source->get_properties().width;
    source_height = cam_source->get_properties().height;

    uint32_t stride = source_width * 4;// YUY2
    const auto selected_shape = [&shape = options.window_shape]() {
        if (shape == "circle") { return bitmap_sink::shape::circle; }

        return bitmap_sink::shape::rectangle;
    }();

    // Create window where the samples will be rendered to
    bitmap_sink::window_ui window_ui{window_width, window_height, stride,
                                     selected_shape};

    // Filters between the media source and sink
    std::vector<std::unique_ptr<sample_filter>> filters;
    // todo: should require source and dest color spaces (YUY is hardcoded)

    ComPtr<ID3D11Device> d3d_device;
    if (enable_hw) { d3d_factory::CreateD3d11Device(); }

    auto sink = create_bitmap_sink(window_ui, d3d_device);

    filters.push_back(create_color_conversion_filter(
            source_width, source_height, d3d_device));

    filters.push_back(create_resizing_filter(source_width, source_height,
                                             1.f * source_width / window_width,
                                             d3d_device));

    auto renderer = std::make_unique<window_renderer>(
            window_renderer::settings{window_width, window_height});

    renderer->bind_render_target(sink->get_swap_chain());
    stream_pipeline pipeline{std::move(cam_source), std::move(filters),
                             std::move(sink)};

    pipeline.start();
    renderer->run();
    pipeline.stop();
}

int main(int argc, char **argv) {
    if (argc <= 1 || strcmp(argv[1], "--h") == 0 ||
        strcmp(argv[1], "--help") == 0) {
        display_cli_help();

        return 0;
    }

    if (strcmp(argv[1], "list-devices") == 0) {
        const auto device_names = camera::list_devices();

        for (const auto &name : device_names) { std::wcout << name << '\n'; }

        return 0;
    }

    if ((strcmp(argv[1], "show") == 0) && argc < 2) {
        display_cli_help();
        return 0;
    }

    CliOptions options;
    options.camera_name = std::string(argv[2]);

    // Extract |show| modifiers
    // todo: invalid or incomplete parameters should abort the program,
    //        right now incomplete options are just being ignored.
    for (int i = 3; i < argc; i++) {
        if (strcmp(argv[i], "--source-size") == 0 && (argc - i) >= 2) {
            options.source_width = std::stoi(argv[i + 1]);
            options.source_height = std::stoi(argv[i + 2]);
        }

        if (strcmp(argv[i], "--window-size") == 0 && (argc - i) >= 2) {
            options.window_width = std::stoi(argv[i + 1]);
            options.window_height = std::stoi(argv[i + 2]);
        }

        if (strcmp(argv[i], "--shape") == 0 && (argc - 1) >= 1) {
            options.window_shape = argv[i + 1];
        }
    }

    try {
        run_pipeline(options);
    } catch (const std::exception &error) {
        std::cerr << error.what() << std::endl;
        return -1;
    }

    return 0;
}
