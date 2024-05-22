#include "utils.h"
#include "tapedelay.h"


//Funkcja obliczająca czas echa dla każdej głowicy
//Proporcjonalnie 1 - 1/4 * delayTime, 2 - 1/2 * delayTime, 3 - 3/4 * delayTime, 4 - delayTime
float CalcDelayTime(float delayTime, int playbackHead){ return delayTime*(float)playbackHead/4.0f; }

//Funkcja wyświetlające parametry echa na ekranie
void HandleDisplay(DaisySeed & hw, MyOledDisplay display, float (& delayParameters)[], int digit)
{    
    display.Fill(false);
    char text[20];
    if(digit == 0) display.DrawRect(0, 0, 18, 28, true, false);
    else if(digit == 1) display.DrawRect(19, 0, 37, 28, true, false);
    else display.DrawRect(38, 0, 56, 28, true, false);
    display.SetCursor(1, 2);
    snprintf(text, sizeof(text), "%d", (int)(delays[3].currentDelay * 10.0f));
    display.WriteString(text, Font_16x26, true);
    display.SetCursor(20, 2);
    snprintf(text, sizeof(text), "%d", (int)(delays[3].currentDelay * 100.0f) % 10);
    display.WriteString(text, Font_16x26, true);
    display.SetCursor(39, 2);
    snprintf(text, sizeof(text), "%d", (int)(delays[3].currentDelay * 1000.0f) % 10);
    display.WriteString(text, Font_16x26, true);
    display.SetCursor(58, 2);
    snprintf(text, sizeof(text), "ms");
    display.WriteString(text, Font_16x26, true);
    display.SetCursor(0, 30);
    float parameterToShow = delayParameters[0]*100.0f;
    snprintf(text, sizeof(text), "Mix: wet %d%%", (int)(parameterToShow));
    display.WriteString(text, Font_6x8, true);
    display.SetCursor(0, 38);
    parameterToShow = delayParameters[1]*100.0f;
    snprintf(text, sizeof(text), "Feedback: %d%%", (int)(parameterToShow));
    display.WriteString(text, Font_6x8, true);
    display.SetCursor(0, 46);
    parameterToShow = delayParameters[3]*100.0f;
    snprintf(text, sizeof(text), "Saturation: %d%%", (int)(parameterToShow));
    display.WriteString(text, Font_6x8, true);
    display.SetCursor(0, 54);
    parameterToShow = delayParameters[4]*1000.0f;
    snprintf(text, sizeof(text), "Wow&Flutter: %d%%", (int)(parameterToShow * 10.0f));
    display.WriteString(text, Font_6x8, true);
    display.Update();
}

//Funkcja sczytująca wybór głowic
bool HandleTapeHeadSelection(GPIO * heads[], uint8_t i)
{
    bool buttonRead = !(heads[i]->Read());
    return buttonRead;
}

//Funkcja przetwarzająca zmiany cyfry na wybranym miejscu dziesiętnym - digit
void changeDecimalPlace(float & origin, int var, float value) {
    int intValue = static_cast<int>(origin * 1000);

    int divisor = 1;
    for(int i = 0; i < 2 - var; ++i){
        divisor *= 10;
    }

    int digit = (intValue / divisor) % 10;
    int newDigit = static_cast<int>(value * 10);

    if(var == 0 && newDigit == 0) newDigit = 1;

    intValue += (newDigit - digit) * divisor;

    origin = static_cast<float>(intValue) / 1000;
}

//Funkcja obsługująca zapisywanie parametrów dźwięku i odczytywanie informacji z przetwornika ADC
void HandlePeripherals(DaisySeed & hw, Delay (& delays)[], GPIO * heads[], float (& delayParameters)[], float (& delayTime)[], bool (& tapeHeadSetup)[], float & startingDelay, int digit)
{
    for(int i = 0; i < 5; i++) delayParameters[i] = hw.adc.GetFloat(i);
    delayParameters[0] *= 1.0101f;
    if(delayParameters[0] > 1.0f) delayParameters[0] = 1.0f;
    delayParameters[1] *= 1.5f;
    delayParameters[3] *= 3.8f;
    delayParameters[3] += 1.2f;
    delayParameters[4] *= 0.0025f;

    changeDecimalPlace(startingDelay, digit, delayParameters[2]);

    for(uint8_t i = 0; i < 4; i++) 
    {
      delayTime[i] = CalcDelayTime(startingDelay, i+1);
      delays[i].currentDelay = delayTime[i];
      tapeHeadSetup[i] = HandleTapeHeadSelection(heads, i);
    }
}