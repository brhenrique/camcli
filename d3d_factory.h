//
// Created by disc on 3/13/2021.
//

#ifndef CAM_CLI_D3D_FACTORY_H
#define CAM_CLI_D3D_FACTORY_H

#include <wrl/client.h>

#include <d3d11_2.h>
#include <mfapi.h>

using Microsoft::WRL::ComPtr;

namespace d3d_factory {
    ComPtr<ID3D11Device> CreateD3d11Device();

    ComPtr<IMFDXGIDeviceManager>
    CreateMfDeviceManager(ID3D11Device *d11_device);
}// namespace d3d_factory

#endif//CAM_CLI_D3D_FACTORY_H
