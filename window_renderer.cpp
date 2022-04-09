//
// Created by disc on 1/17/2021.
//

#define NOMINMAX 1

#pragma comment(lib, "d2d1")
#pragma comment(lib, "dxgi")
#pragma comment(lib, "d3d11")
#pragma comment(lib, "Dcomp")
#pragma comment(lib, "strmiids")

#include "window_renderer.h"

#include <dcomp.h>
#include <mfidl.h>

#include <string>

#include "utils.h"

EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define HINST_THISCOMPONENT ((HINSTANCE) &__ImageBase)

window_renderer::window_renderer(settings s) : hwnd_(nullptr), settings_(s) {
    create_window();
}

void window_renderer::run() {
    MSG msg;

    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

LRESULT window_renderer::window_proc(HWND hwnd, uint32_t message, WPARAM wparam,
                                     LPARAM lparam) {
    if (message == WM_CREATE) {
        auto pcs = reinterpret_cast<LPCREATESTRUCT>(lparam);
        auto win = reinterpret_cast<window_renderer *>(pcs->lpCreateParams);

        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<intptr_t>(win));

        return 1;
    }

    int result = 0;

    switch (message) {
        case WM_SIZE:
            result = 0;
            break;
        case WM_DISPLAYCHANGE:
            InvalidateRect(hwnd, nullptr, FALSE);
            result = 0;
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            result = 1;
            break;
        case WM_NCHITTEST: {
            LRESULT hit = DefWindowProc(hwnd, message, wparam, lparam);
            if (hit == HTCLIENT) hit = HTCAPTION;
            return hit;
        }
        default:
            result = DefWindowProc(hwnd, message, wparam, lparam);
            break;
    }

    return result;
}

void window_renderer::close() {
    if (hwnd_) { DestroyWindow(hwnd_); }
}

// configuration values hardcoded
void window_renderer::create_window() {
    WNDCLASSEX wcex = {0};
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = window_renderer::window_proc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = sizeof(LONG_PTR);
    wcex.hInstance = HINST_THISCOMPONENT;
    wcex.hbrBackground = nullptr;
    wcex.lpszMenuName = nullptr;
    wcex.hCursor = LoadCursor(nullptr, IDI_APPLICATION);
    wcex.lpszClassName = "Camera preview";

    if (!RegisterClassExA(&wcex)) {
        std::cout << "RegisterClass failed: " << GetLastError() << std::endl;
        throw std::runtime_error("failed to register win class");
    }

    // WS_EX_NOREDIRECTIONBITMAP to bind the window to the composition device.
    hwnd_ = CreateWindowEx(WS_EX_NOREDIRECTIONBITMAP, wcex.lpszClassName,
                           "Camera preview", WS_POPUP, 0, 0,
                           static_cast<uint32_t>(settings_.width),
                           static_cast<uint32_t>(settings_.height), nullptr,
                           nullptr, HINST_THISCOMPONENT, this);

    if (!hwnd_) {
        std::cout << "win32 error: " << GetLastError() << std::endl;
        throw std::runtime_error("failed to create window_renderer: ");
    }

    SetWindowPos(hwnd_, nullptr, 0, 0, 0, 0,
                 SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
                         SWP_NOOWNERZORDER);
    ShowWindow(hwnd_, SW_SHOWNORMAL);
    UpdateWindow(hwnd_);
}

void window_renderer::bind_render_target(IDXGISwapChain1 *swap_chain) {
    ComPtr<IDXGIDevice> dxgi_device;

    // Extract device that owns the swap chain
    THROW_IF_FAILED(
            swap_chain->GetDevice(IID_PPV_ARGS(dxgi_device.GetAddressOf())));

    THROW_IF_FAILED(DCompositionCreateDevice(
            dxgi_device.Get(), __uuidof(dcomp_device_),
            reinterpret_cast<void **>(dcomp_device_.GetAddressOf())));

    // Bind composition device to window, and swap chain to composition device.
    THROW_IF_FAILED(dcomp_device_->CreateTargetForHwnd(hwnd_,
                                                       TRUE,// Top most
                                                       target_.GetAddressOf()));
    THROW_IF_FAILED(dcomp_device_->CreateVisual(visual_.GetAddressOf()));
    THROW_IF_FAILED(visual_->SetContent(swap_chain));
    THROW_IF_FAILED(target_->SetRoot(visual_.Get()));
    THROW_IF_FAILED(dcomp_device_->Commit());
}

window_renderer::~window_renderer() { close(); }
