//
// Created by disc on 1/16/2021.
//

#ifndef CAM_CLI_CAMERA_H
#define CAM_CLI_CAMERA_H

#include <mfidl.h>
#include <wrl/client.h>

#include <string>
#include <vector>

using Microsoft::WRL::ComPtr;

// make it explicit that this is a factory
namespace camera {
    std::vector<std::wstring> list_devices();

    /**
     *  Creates a new IMFMediaSource connected to the camera device specified.
     *  The |device_name| should be equal to the MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME attribute.
     *
     *  Returns an empty ComPtr if no device was found with the supplied name
     */
    ComPtr<IMFMediaSource> create_media_source(const std::wstring &device_name);
};// namespace camera


#endif//CAM_CLI_CAMERA_H
