//
// Created by disc on 1/16/2021.
//

#include <mfapi.h>
#include <mfidl.h>

#include <vector>

#include "camera.h"
#include "utils.h"

namespace camera {
    ComPtr<IMFMediaSource>
    create_media_source(const std::wstring &device_name) {
        IMFActivate **devices;
        ComPtr<IMFAttributes> config;

        THROW_IF_FAILED(MFCreateAttributes(&config, 1));
        THROW_IF_FAILED(config->SetGUID(
                MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE,
                MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID));

        uint32_t count = 0;
        THROW_IF_FAILED(MFEnumDeviceSources(config.Get(), &devices, &count));

        if (count == 0) { return {}; }

        for (uint32_t i = 0; i < count; i++) {
            ComPtr<IMFMediaSource> source;

            wchar_t *friendly_name;
            uint32_t name_length = 0;

            THROW_IF_FAILED(devices[i]->GetAllocatedString(
                    MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME, &friendly_name,
                    &name_length));

            if (wcscmp(friendly_name, device_name.c_str()) == 0) {
                // todo: catch error when device is busy, return empty ComPtr
                THROW_IF_FAILED(
                        devices[i]->ActivateObject(IID_PPV_ARGS(&source)));
                return source;
            }
        }

        return {};
    }

    std::vector<std::wstring> list_devices() {
        IMFActivate **devices;// todo: will leak
        ComPtr<IMFAttributes> config;

        THROW_IF_FAILED(MFCreateAttributes(&config, 1));
        THROW_IF_FAILED(config->SetGUID(
                MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE,
                MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID));

        uint32_t count = 0;

        THROW_IF_FAILED(MFEnumDeviceSources(config.Get(), &devices, &count));
        std::vector<std::wstring> device_names;

        for (uint32_t i = 0; i < count; i++) {
            wchar_t *friendly_name;
            uint32_t name_length = 0;

            THROW_IF_FAILED(devices[i]->GetAllocatedString(
                    MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME, &friendly_name,
                    &name_length));


            device_names.emplace_back(friendly_name, name_length);
        }

        return device_names;
    };
}// namespace camera
