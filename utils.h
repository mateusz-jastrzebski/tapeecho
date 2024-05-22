#pragma once
#include "tapedelay.h"

using namespace daisy;
using namespace daisy::seed;
using namespace daisysp;


//Deklaracja funkcji
float CalcDelayTime(float delayTime, int playbackHead);
void HandleDisplay(DaisySeed & hw, MyOledDisplay display, float (& delayParameters)[], int digit);
bool HandleTapeHeadSelection(GPIO * heads[], uint8_t i, bool (& previousButtonState)[]);
void HandlePeripherals(DaisySeed & hw, Delay (& delays)[], GPIO * heads[], float (& delayParameters)[], float (& delayTime)[], bool (& tapeHeadSetup)[], float & startingDelay, int digit);