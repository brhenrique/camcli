//
// Created by disc on 1/17/2021.
//

#ifndef CAM_CLI_WINDOW_RENDERER_H
#define CAM_CLI_WINDOW_RENDERER_H

#include <wrl/client.h>

#include <cstdint>

class IDCompositionDevice;
class IDCompositionVisual;
class IDCompositionTarget;
class IDXGIDevice;
class IDXGISwapChain1;

using Microsoft::WRL::ComPtr;

/**
 * Create a win32 window bound to a an IDXGISwapChain.
 * It can be used to the |bitmap_sink| class to render
 * samples written to it.
 */
class window_renderer {
public:
    struct settings {
        uint32_t width;
        uint32_t height;
    };

    window_renderer(settings s);

    ~window_renderer();

    void run();

    void close();

    void bind_render_target(IDXGISwapChain1 *swap_chain);

private:
    static LRESULT CALLBACK window_proc(HWND hwnd, uint32_t message,
                                        WPARAM wparm, LPARAM lparam);

    void create_window();

    // Composition
    ComPtr<IDCompositionDevice> dcomp_device_;
    ComPtr<IDCompositionVisual> visual_;
    ComPtr<IDCompositionTarget> target_;

    HWND hwnd_;
    settings settings_;
};


#endif//CAM_CLI_WINDOW_RENDERER_H
