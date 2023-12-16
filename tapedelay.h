#pragma once
#include "daisysp.h"
#include "daisy_seed.h"
#include "daisy_core.h"
#include "math.h"
#include "dev/oled_ssd130x.h"
#include <string>


using namespace daisy;
using namespace daisy::seed;
using namespace daisysp;

using MyOledDisplay = OledDisplay<SSD130xI2c128x32Driver>;

#define LEFT (i)
#define RIGHT (i + 1)

#define MAX_DELAY static_cast<size_t>(96000 * 1.0f)

struct Delay {
    DelayLine<float, MAX_DELAY> *delay;
    float currentDelay;
    float delayTarget;

    float ProcessDelay(float in, uint8_t i);
};

extern Delay delays[4];