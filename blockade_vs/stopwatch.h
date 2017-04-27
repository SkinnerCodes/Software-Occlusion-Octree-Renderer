#pragma once
#include "VoxelRenderBase.h"
#include <windows.h>

class StopWatch {
    LARGE_INTEGER frequency_;
    LARGE_INTEGER startTime_;
    LARGE_INTEGER stopTime_;

public:
    StopWatch();

    void Start();
    void Stop();

    __int64 MilliSeconds() const;

};