#pragma once
#include "daisysp.h"
#include "daisy_seed.h"
#include "daisy_core.h"
#include "tapedelay.h"



using namespace daisy;
using namespace daisy::seed;
using namespace daisysp;

float IntToFloat(uint16_t intToConvert);
float CalcDelayTime(float delayTime, int playbackHead);
float RoundFloat(float number, int d_places=0);
uint16_t Smoothen(DaisySeed & hw, int pin, int numberOfSamples);
void HandleDisplay(DaisySeed & hw, float (& delayParameters)[], float (& previousDelayParameters)[]);
bool HandleTapeHeadSelection(GPIO * heads[], uint8_t i, bool tapeHead, bool (& previousButtonState)[]);
void HandlePeripherals(DaisySeed & hw, Delay (& delays)[], GPIO * heads[], float (& delayParameters)[], float (& previousDelayParameters)[], float (& delayTime)[], bool (& tapeHeadSetup)[], bool (& previousButtonsState)[]);