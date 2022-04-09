//
// Created by disc on 3/13/2021.
//

#pragma comment(lib, "d2d1")
#pragma comment(lib, "dxgi")
#pragma comment(lib, "d3d11")
#pragma comment(lib, "strmiids")

#include "d3d_factory.h"

#include <d2d1.h>
#include <mfidl.h>

#include "utils.h"

namespace d3d_factory {
    ComPtr<ID3D11Device> CreateD3d11Device() {
        ComPtr<ID3D11Device> d11_device;

        THROW_IF_FAILED(
                D3D11CreateDevice(nullptr,// Adapter
                                  D3D_DRIVER_TYPE_HARDWARE,
                                  nullptr,// Module
                                  D3D11_CREATE_DEVICE_BGRA_SUPPORT |
                                          D3D11_CREATE_DEVICE_VIDEO_SUPPORT |
                                          D3D11_CREATE_DEVICE_DEBUG,
                                  nullptr,
                                  0,// Highest available feature level
                                  D3D11_SDK_VERSION, &d11_device,
                                  nullptr,// Actual feature level
                                  nullptr));

        ComPtr<ID3D10Multithread> d3d_threading;
        d11_device.As(&d3d_threading);
        d3d_threading->SetMultithreadProtected(TRUE);

        return d11_device;
    }

    ComPtr<IMFDXGIDeviceManager>
    CreateMfDeviceManager(ID3D11Device *d11_device) {
        ComPtr<IMFDXGIDeviceManager> mf_manager;
        uint32_t reset_token;
        THROW_IF_FAILED(MFCreateDXGIDeviceManager(&reset_token, &mf_manager));
        THROW_IF_FAILED(mf_manager->ResetDevice(d11_device, reset_token));

        return mf_manager;
    }
}// namespace d3d_factory