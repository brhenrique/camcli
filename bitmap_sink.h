//
// Created by disc on 3/14/2021.
//

#ifndef CAM_CLI_BITMAP_SINK_H
#define CAM_CLI_BITMAP_SINK_H

#include <d2d1_2.h>
#include <d3d11.h>
#include <dxgi.h>
#include <dxgi1_2.h>

#include <cstdint>

#include "sample_sink.h"

class bitmap_sink : public sample_sink {
public:
    enum shape { rectangle, circle };

    struct window_ui {
        uint32_t width;
        uint32_t height;
        uint32_t stride;
        shape shape;
    };

    bitmap_sink(window_ui ui, ComPtr<ID3D11Device> d3d_device = nullptr);

    void write(Sample sample) override;

    IDXGISwapChain1 *get_swap_chain() const;

private:
    static const DXGI_FORMAT d3d_pixel_format = DXGI_FORMAT_B8G8R8A8_UNORM;

    void apply_circle_mask();

    void create_d2d1_device();

    void create_d3d_device();

    void create_swap_chain();

    void create_render_target();

    // DXGI
    ComPtr<IDXGIDevice> dxgi_device_;
    ComPtr<IDXGISwapChain1> swap_chain_;
    ComPtr<IDXGISurface2> surface_;
    // D3D11
    ComPtr<ID3D11Device> d3d_device_;
    // D2D1
    ComPtr<ID2D1Factory2> d2d1_factory_;
    ComPtr<ID2D1DeviceContext> dc_;
    ComPtr<ID2D1Bitmap1> bitmap_;
    ComPtr<ID2D1Bitmap1> src_bitmap_;
    ComPtr<ID2D1Device1> d2d1_device_;
    ComPtr<ID2D1PathGeometry> path_geometry_;

    window_ui ui_;
};


#endif//CAM_CLI_BITMAP_SINK_H
