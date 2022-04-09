//
// Created by disc on 3/6/2021.
//

#ifndef CAM_CLI_SAMPLE_SINK_H
#define CAM_CLI_SAMPLE_SINK_H

#include "sample.h"

class IMFSample;

class sample_sink {
public:
    virtual void write(Sample sample) = 0;
};


#endif//CAM_CLI_SAMPLE_SINK_H
