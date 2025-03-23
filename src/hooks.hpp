#pragma once

static float* g_deltaTime = (float*)REL::RelocationID(523660, 410199).address();
static float* g_deltaTimeRealTime = (float*)REL::RelocationID(523661, 410200).address();   

namespace hooks
{
    void install();
}