#include "utils.h"
#include "tapedelay.h"


float IntToFloat(uint16_t intToConvert) { return ((float)intToConvert / 65535.0f); }

float RoundFloat(float number, int d_places){ return (d_places==2)?(std::round(number * 1000.0f) / 1000.0f):(std::round(number * 100.0f) / 100.0f); }

float CalcDelayTime(float delayTime, int playbackHead){ return delayTime*(float)playbackHead/4.0f; }

uint16_t Smoothen(DaisySeed & hw, int pin, int numberOfSamples)
{
    uint16_t samples[numberOfSamples];

    for(int i = 0; i < numberOfSamples; i++)
    {
        samples[i] = hw.adc.Get(pin);
    }

    std::sort(samples, samples + (sizeof(samples) / sizeof(samples[0])));

    return (numberOfSamples % 2 == 0) ? (samples[numberOfSamples/2 - 1] + samples[numberOfSamples/2]) / 2 : samples[numberOfSamples/2];
}

void HandleDisplay(DaisySeed & hw, float (& delayParameters)[], float (& previousDelayParameters)[])
{    
    return;
}

bool HandleTapeHeadSelection(GPIO * heads[], uint8_t i, bool tapeHead, bool (& previousButtonState)[])
{
    bool buttonRead = !(heads[i]->Read());

    if(buttonRead) { previousButtonState[i] = 1; return tapeHead; }
    if(previousButtonState[i] == 1 && !buttonRead) { previousButtonState[i] = 0; return !tapeHead; }
    return tapeHead;
}

void HandlePeripherals(DaisySeed & hw, Delay (& delays)[], GPIO * heads[], float (& delayParameters)[], float (& previousDelayParameters)[], float (& delayTime)[], bool (& tapeHeadSetup)[], bool (& previousButtonsState)[])
{
    //for(int i = 0; i < 3; i++) delayParameters[i] = RoundFloat(IntToFloat(hw.adc.Get(i)), i);
    for(int i = 0; i < 3; i++) delayParameters[i] = hw.adc.GetFloat(i);
    for(uint8_t i = 0; i < 4; i++) 
    {
      delayTime[i] = CalcDelayTime(delayParameters[2], i+1);
      delays[i].delayTarget = delayTime[i];
      tapeHeadSetup[i] = HandleTapeHeadSelection(heads, i, tapeHeadSetup[i], previousButtonsState);
    }
}