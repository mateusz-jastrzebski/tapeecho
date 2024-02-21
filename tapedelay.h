#pragma once
#include "daisysp.h"
#include "daisy_seed.h"
#include "daisy_core.h"
#include "math.h"
#include "dev/oled_ssd130x.h"


using namespace daisy;
using namespace daisy::seed;
using namespace daisysp;

using MyOledDisplay = OledDisplay<SSD130xI2c128x64Driver>;

#define LEFT (i)
#define RIGHT (i + 1)

#define MAX_DELAY static_cast<size_t>(48000 * 1.5f)

struct Delay {
    DelayLine<float, MAX_DELAY> *delay;
    float currentDelay;

    float ProcessDelay(float in, float wowValue, float flutterValue, float depth, uint8_t i);
};

extern Delay delays[4];