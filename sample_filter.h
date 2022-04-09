//
// Created by disc on 3/6/2021.
//

#ifndef CAM_CLI_SAMPLE_FILTER_H
#define CAM_CLI_SAMPLE_FILTER_H

#include "sample.h"

class sample_filter {
public:
    virtual Sample process(Sample input) = 0;
};

#endif//CAM_CLI_SAMPLE_FILTER_H
