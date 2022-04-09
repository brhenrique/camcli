//
// Created by disc on 3/7/2021.
//
#pragma comment(lib, "d2d1")
#pragma comment(lib, "dxguid")

#include "resizer.h"

#include <d2d1_1.h>
#include <d2d1effects.h>
#include <mfapi.h>

ComPtr<IMFSample> resizer::process(ComPtr<IMFSample> input) {
    effect_->SetInput(0, bitmap_.Get());
    effect_->SetValue(D2D1_SCALE_PROP_CENTER_POINT,
                      D2D1::Vector2F(256.0f, 192.0f));
    effect_->SetValue(D2D1_SCALE_PROP_SCALE, D2D1::Vector2F(2.0f, 2.0f));

    dc_->BeginDraw();
    dc_->DrawImage(effect_.Get());
    dc_->EndDraw();

    return ComPtr<IMFSample>();
}

resizer::resizer(size_t in_width, size_t in_height, float scale_factor)
    : in_width_(in_width), in_height_(in_height), scale_factor_() {
    dc_->CreateEffect(CLSID_D2D1Scale, &effect_);
}
