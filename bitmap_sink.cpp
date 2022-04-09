//
// Created by disc on 3/14/2021.
//

#pragma comment(lib, "d2d1")
#pragma comment(lib, "dxgi")
#pragma comment(lib, "d3d11")
#pragma comment(lib, "Dcomp")
#pragma comment(lib, "strmiids")

#include "bitmap_sink.h"

#include <dcomp.h>
#include <dcomptypes.h>
#include <dxgi1_3.h>
#include <mfidl.h>

#include <string>

#include "d3d_factory.h"
#include "utils.h"
#include "win_graphics.h"

bitmap_sink::bitmap_sink(window_ui ui, ComPtr<ID3D11Device> d3d_device)
    : ui_(ui), d3d_device_(d3d_device) {
    create_d3d_device();
    create_swap_chain();
    create_d2d1_device();
    create_render_target();

    if (ui_.shape == shape::circle) { apply_circle_mask(); }

    D2D1_BITMAP_PROPERTIES1 properties = {};
    properties.pixelFormat.alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED;
    properties.pixelFormat.format = d3d_pixel_format;
    properties.bitmapOptions = D2D1_BITMAP_OPTIONS_NONE;

    THROW_IF_FAILED(dc_->CreateBitmap(D2D1::SizeU(ui_.width, ui_.height),
                                      nullptr, ui_.stride, &properties,
                                      src_bitmap_.GetAddressOf()));
}

//void bitmap_sink::write(Sample sample) {
//    SampleToBitmap(sample, src_bitmap_, ui_.width, ui_.height, ui_.stride);
//
//    dc_->BeginDraw();
//    dc_->Clear();
//
//    auto transforms = D2D1::Matrix3x2F::Scale(
//            D2D1::SizeF(ui_.scale_factor, -1 * ui_.scale_factor),
//            D2D1::Point2F(0, ui_.height / 2));
//
//    if (ui_.shape == shape::circle) {
//        int64_t offset = -1 *
//                         (static_cast<int64_t>(ui_.width) -
//                          static_cast<int64_t>(ui_.height)) /
//                         2;
//        transforms = transforms *
//                     D2D1::Matrix3x2F::Translation(D2D1::SizeF(offset, 0));
//    }
//
//    dc_->SetTransform(transforms);
//    dc_->PushLayer(
//            D2D1::LayerParameters(D2D1::InfiniteRect(), path_geometry_.Get()),
//            nullptr);
//
//    dc_->DrawBitmap(src_bitmap_.Get(),
//                    D2D_RECT_F{0, 0, static_cast<FLOAT>(ui_.width),
//                               static_cast<FLOAT>(ui_.height)},
//                    1, D2D1_INTERPOLATION_MODE_CUBIC, nullptr);
//    dc_->PopLayer();
//
//    THROW_IF_FAILED(dc_->EndDraw());
//    THROW_IF_FAILED(swap_chain_->Present(1, 0));
//}

void bitmap_sink::write(Sample sample) {
    bool use_mirror = false;
    ComPtr<ID2D1Bitmap> bitmap = GpuSampleToBitmap(sample, dc_);

    if (!bitmap) {
        // Sample is not in the GPU memory, copy from system
        SampleToBitmap(sample, src_bitmap_, ui_.width, ui_.height, ui_.stride);
        bitmap = src_bitmap_;
        use_mirror = true;
    }

    dc_->BeginDraw();
    dc_->Clear();

    auto transforms = D2D1::IdentityMatrix();

    if (use_mirror) {
        // First apply a translation at the y axis equal to the image height, then
        // apply a scale transform to invert it vertically
        transforms = transforms *
                     D2D1::Matrix3x2F::Translation(D2D1::SizeF(0, ui_.height));
        transforms = transforms *
                     D2D1::Matrix3x2F::Scale(D2D1::SizeF(1, -1),
                                             D2D1::Point2F(0, ui_.height));
    }

    if (ui_.shape == shape::circle) {
        // Display center of the frame inside the circle, the first pixel
        // to be displayed will be located at (width - height) / 2.
        int64_t offset = -1 *
                         (static_cast<int64_t>(ui_.width) -
                          static_cast<int64_t>(ui_.height)) /
                         2;
        transforms = transforms *
                     D2D1::Matrix3x2F::Translation(D2D1::SizeF(offset, 0));
    }

    dc_->SetTransform(transforms);

    if (path_geometry_) {
        dc_->PushLayer(D2D1::LayerParameters(D2D1::InfiniteRect(),
                                             path_geometry_.Get()),
                       nullptr);
    }

    dc_->DrawBitmap(bitmap.Get(),
                    D2D_RECT_F{0, 0, static_cast<FLOAT>(ui_.width),
                               static_cast<FLOAT>(ui_.height)},
                    1, D2D1_INTERPOLATION_MODE_CUBIC, nullptr);

    if (path_geometry_) { dc_->PopLayer(); }

    THROW_IF_FAILED(dc_->EndDraw());
    THROW_IF_FAILED(swap_chain_->Present(1, 0));
};

void bitmap_sink::apply_circle_mask() {
    auto radius = ui_.height / 2;

    THROW_IF_FAILED(d2d1_factory_->CreatePathGeometry(&path_geometry_));
    ComPtr<ID2D1GeometrySink> sink;
    THROW_IF_FAILED(path_geometry_->Open(&sink));
    sink->SetFillMode(D2D1_FILL_MODE_WINDING);

    auto x_offset = (ui_.width - ui_.height) / 2;
    sink->BeginFigure(D2D1::Point2F(x_offset, radius),
                      D2D1_FIGURE_BEGIN_FILLED);

    sink->AddArc(D2D1::ArcSegment(D2D1::Point2F(x_offset + 2 * radius, radius),
                                  D2D1::SizeF(radius, radius), 0.0f,
                                  D2D1_SWEEP_DIRECTION_CLOCKWISE,
                                  D2D1_ARC_SIZE_SMALL));

    // Add the bottom half circle
    sink->AddArc(D2D1::ArcSegment(
            D2D1::Point2F(x_offset,
                          radius),// end point of the bottom half circle
            D2D1::SizeF(
                    radius,
                    radius),// radius of the bottom half circle, same as previous one.
            0.0f,           // rotation angle
            D2D1_SWEEP_DIRECTION_CLOCKWISE, D2D1_ARC_SIZE_SMALL));

    sink->EndFigure(D2D1_FIGURE_END_CLOSED);
    THROW_IF_FAILED(sink->Close());
}

void bitmap_sink::create_d2d1_device() {
    THROW_IF_FAILED(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED,
                                      {D2D1_DEBUG_LEVEL_INFORMATION},
                                      d2d1_factory_.GetAddressOf()));
    THROW_IF_FAILED(d2d1_factory_->CreateDevice(dxgi_device_.Get(),
                                                d2d1_device_.GetAddressOf()));
    THROW_IF_FAILED(d2d1_device_->CreateDeviceContext(
            D2D1_DEVICE_CONTEXT_OPTIONS_NONE, dc_.GetAddressOf()));
}

void bitmap_sink::create_d3d_device() {
    if (!d3d_device_) { d3d_device_ = d3d_factory::CreateD3d11Device(); }
    THROW_IF_FAILED(d3d_device_.As(&dxgi_device_));
}

/**
 * Creates:
 *  - swap_chain_
 *  - swap_chain_handle_
 */
void bitmap_sink::create_swap_chain() {
    ComPtr<IDXGIFactory2> dx_factory;
    HANDLE swap_chain_handle;

    THROW_IF_FAILED(CreateDXGIFactory2(
            DXGI_CREATE_FACTORY_DEBUG, __uuidof(dx_factory),
            reinterpret_cast<void **>(dx_factory.GetAddressOf())));

    THROW_IF_FAILED(DCompositionCreateSurfaceHandle(
            COMPOSITIONOBJECT_ALL_ACCESS, nullptr, &swap_chain_handle));

    DXGI_SWAP_CHAIN_DESC1 description = {0};
    description.Format = d3d_pixel_format;
    description.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    description.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
    description.BufferCount = 2;
    description.SampleDesc.Count = 1;
    description.AlphaMode = DXGI_ALPHA_MODE_PREMULTIPLIED;
    description.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
    description.Width = ui_.width;
    description.Height = ui_.height;

    ComPtr<IDXGIFactoryMedia> dxgi_media;
    dx_factory.As(&dxgi_media);

    THROW_IF_FAILED(dxgi_media->CreateSwapChainForCompositionSurfaceHandle(
            d3d_device_.Get(), swap_chain_handle, &description, nullptr,
            swap_chain_.GetAddressOf()));
}

void bitmap_sink::create_render_target() {
    THROW_IF_FAILED(swap_chain_->GetBuffer(
            0, __uuidof(surface_),
            reinterpret_cast<void **>(surface_.GetAddressOf())));
    D2D1_BITMAP_PROPERTIES1 properties = {};
    properties.pixelFormat.alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED;
    properties.pixelFormat.format = d3d_pixel_format;
    properties.bitmapOptions =
            D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW;

    THROW_IF_FAILED(dc_->CreateBitmapFromDxgiSurface(surface_.Get(), properties,
                                                     bitmap_.GetAddressOf()));
    dc_->SetTarget(bitmap_.Get());
}

IDXGISwapChain1 *bitmap_sink::get_swap_chain() const {
    return swap_chain_.Get();
}
