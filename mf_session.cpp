//
// Created by disc on 1/29/2021.
//

#include <mfapi.h>

#include "mf_session.h"

mf_session::mf_session() {
    CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    MFStartup(MF_VERSION);
}

mf_session::~mf_session() {
    MFShutdown();
    CoUninitialize();
}
