//
// Created by disc on 1/16/2021.
//

#ifndef CAM_CLI_UTILS_H
#define CAM_CLI_UTILS_H

#include <iostream>
#include <system_error>

#define THROW_IF_FAILED(hr)                                                    \
    {                                                                          \
        if (FAILED(hr)) {                                                      \
            auto error = get_hr_error_message(hr);                             \
            throw std::runtime_error(error);                                   \
        }                                                                      \
    }

#define RETURN_IF_FAILED(hr)                                                   \
    {                                                                          \
        if (FAILED(hr)) {                                                      \
            auto error = get_hr_error_message(hr);                             \
            std::cout << "HRESULT error: " << error << std::endl;              \
            return {};                                                         \
        }                                                                      \
    }

inline std::string get_hr_error_message(HRESULT hr) {
    return std::system_category().message(hr);
}

#endif//CAM_CLI_UTILS_H
