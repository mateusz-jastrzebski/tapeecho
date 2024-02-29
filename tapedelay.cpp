#include "tapedelay.h"
#include "utils.h"
#include "lfo.h"

//Deklaracja zmiennych globalnych
Svf lowPass;
DaisySeed hw;
MyOledDisplay display;
static AdcChannelConfig adc_config[5];

LFO wow(0.25f, 48000.0f);
LFO flutter(15.0f, 48000.0f);


GPIO fLED;
GPIO sLED;
GPIO tLED;
GPIO foLED;
GPIO OnOffLED;

GPIO* led[5] = {&fLED, &sLED, &tLED, &foLED, &OnOffLED};

GPIO firstHead;
GPIO secondHead;
GPIO thirdHead;
GPIO fourthHead;
GPIO OnOffSwitch;

GPIO* heads[4] = {&firstHead, &secondHead, &thirdHead, &fourthHead};


uint8_t activeHeadCount = 0;
uint8_t digit = 0;

DelayLine<float, MAX_DELAY> DSY_SDRAM_BSS sdramDelays[4];

bool activeHeads[4];
float delayParameters[5];
float startingDelay = 0.312f;

bool turnedOff;
float delay_out[4];
float sample_rate;

float delayTime[4];


Delay delays[4];

//Funkcja realizująca emulację saturacji taśmy magnetycznej
float TapeSaturation(float x, float drive) {
    return tanhf(drive * x + 0.24f);
}

//Funkcja struktury Delay realizująca ustawianie echa
float Delay::ProcessDelay(float in, float wowValue, float flutterValue, float depth, uint8_t i)
{
  delay->SetDelay(sample_rate * currentDelay * (1.0f + wowValue * 2.0f * depth + flutterValue * 0.2f * depth));
  float read = delay->Read();
  delay->Write(delayParameters[1] * read + in);
  return read;
};

//Funkcja obsługująca zczytywanie odpowiednich buforów echa w zależności od wybranych głowic
void HandleDelays(float in, bool tapeHeadSetup[])
{
  activeHeadCount = 0;
  float wowValue = wow.Process();
  float flutterValue = flutter.Process();
  float depth = delayParameters[4];

  for(int i = 0; i < 4; i++) {
    if(tapeHeadSetup[i])
    {
      led[i]->Write(true);
      delay_out[i] = delays[i].ProcessDelay(in, wowValue, flutterValue, depth, i);
      activeHeadCount++;
    }
    else
    {
      led[i]->Write(false);
      delay_out[i] = 0.0f;
    }
  }
  led[4]->Write(!turnedOff);
}


//Funkcja obsługująca całe audio
void AudioCallback(AudioHandle::InterleavingInputBuffer in,
                   AudioHandle::InterleavingOutputBuffer out,
                   size_t                    size)
{
    //Obsługa peryferiów
    HandlePeripherals(hw, delays, heads, delayParameters, delayTime, activeHeads, startingDelay, digit);
    float altered_x = 0.0f;    

    for(size_t i = 0; i < size; i+=2)
    {
        float audio_out = 0.0f;
        
        //Przetwarzanie filtru dolnoprzepustowego
        lowPass.Process(in[LEFT]);
        altered_x = lowPass.Low();
        //Przetwarzanie saturacji taśmy magnetycznej
        altered_x = TapeSaturation(altered_x, delayParameters[3]);
        //Przetwarzanie echa
        HandleDelays(altered_x, activeHeads);
        //Miksowanie sygnałów
        if(activeHeadCount == 0 || turnedOff) audio_out = in[LEFT];
        else
        {
          for(int j = 0; j < 4; j++) audio_out += delay_out[j] / activeHeadCount;
          audio_out *= delayParameters[0];
          audio_out += (1.0f - delayParameters[0])*in[LEFT];
        }
        
        //Ograniczenie maksymalnej wartości amplitudy na wyjściu
        if(audio_out > 1.0f) audio_out = 1.0f;
        else if(audio_out < -1.0f) audio_out = -1.0f;

        //Wyjście sygnału
        out[LEFT] = audio_out;
        out[RIGHT] = in[LEFT];
    }    
}



int main(void)
{
  //Inicjalizacja urządzenia, ustawienie częstotliwości próbkowania i rozmiaru próbek
  hw.Init();
  hw.SetAudioBlockSize(256);
  hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);
  sample_rate = hw.AudioSampleRate();

  //Konfiguracja i inicjalizacja wyświetlacza OLED
  MyOledDisplay::Config disp_cfg;
  disp_cfg.driver_config.transport_config.i2c_address               = 0x3C;
  disp_cfg.driver_config.transport_config.i2c_config.periph         = I2CHandle::Config::Peripheral::I2C_1;
  disp_cfg.driver_config.transport_config.i2c_config.speed          = I2CHandle::Config::Speed::I2C_100KHZ;
  disp_cfg.driver_config.transport_config.i2c_config.mode           = I2CHandle::Config::Mode::I2C_MASTER;
  disp_cfg.driver_config.transport_config.i2c_config.pin_config.scl = {DSY_GPIOB, 8};    disp_cfg.driver_config.transport_config.i2c_config.pin_config.sda = {DSY_GPIOB, 9};
  display.Init(disp_cfg);

  //Inicjalizacja filtra dolnoprzepustowego
  lowPass.Init(sample_rate);
  lowPass.SetFreq(6000.0f);
  lowPass.SetRes(0.0f);
  lowPass.SetDrive(0.0f);

  //Inicjalizacja diod LED
  fLED.Init(D21, GPIO::Moe::OUTPUT);
  sLED.Init(D22, GPIO::Mode::OUTPUT);
  tLED.Init(D23, GPIO::Mode::OUTPUT);
  foLED.Init(D24, GPIO::Mode::OUTPUT);
  OnOffLED.Init(D25, GPIO::Mode::OUTPUT);

  //Inicjalizacja przełączników
  firstHead.Init(D10, GPIO::Mode::INPUT, GPIO::Pull::PULLUP);
  secondHead.Init(D9, GPIO::Mode::INPUT, GPIO::Pull::PULLUP);
  thirdHead.Init(D8, GPIO::Mode::INPUT, GPIO::Pull::PULLUP);
  fourthHead.Init(D7, GPIO::Mode::INPUT, GPIO::Pull::PULLUP);
  OnOffSwitch.Init(D13, GPIO::Mode::INPUT, GPIO::Pull::PULLUP);

  //Inicjalizacja potencjometrów
  adc_config[0].InitSingle(A4); //    DRY / WET
  adc_config[1].InitSingle(A3); //    REPEATS
  adc_config[2].InitSingle(A2); //    DELAY TIME
  adc_config[3].InitSingle(A1); //    TAPE SATURATION
  adc_config[4].InitSingle(A0); //    WOW & FLUTTER

  hw.adc.Init(adc_config, 5);
  hw.adc.Start();
  
  //Wstępna inicjalizacja oraz konfiguracja echa
  for(uint8_t i = 0; i < 4; i++)
  {
    sdramDelays[i].Init();
    delays[i].delay = &sdramDelays[i];
    delays[i].currentDelay = delayParameters[2];
  }

  //Rozpoczęcie przetwarzanie audio
  hw.StartAudio(AudioCallback);

  //Zmienne pomocnicze
  bool lastReading = activeHeads[0];
  uint8_t clickCount = 0;
  auto start_time = System::GetNow();
  auto duration = System::GetNow() - start_time;

  for(;;){
    //Obsługa przycisku odpowiadającego za Bypass
    turnedOff = OnOffSwitch.Read();
    
    //Obsługa podwójnego kliku
    if(duration > 10000) duration = 5000;
    else duration = System::GetNow() - start_time;

    if(activeHeads[0] != lastReading){
        if (duration < 800) {
            clickCount++;
            if (clickCount == 2) {
                if(digit + 1 <= 2) digit++;
                else digit = 0;
                clickCount = 0;
                start_time = System::GetNow();
            }
        } else {
            clickCount = 1;
            start_time = System::GetNow();
        }
        lastReading = activeHeads[0];
    }

    //Obsługa wyświetlacza OLED
    HandleDisplay(hw, display, delayParameters, digit);
  }
}
