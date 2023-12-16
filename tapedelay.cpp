#include "tapedelay.h"
#include "utils.h"


DaisySeed hw;
MyOledDisplay display;
CpuLoadMeter loadMeter;
//GPIO tap_tempo;
static AdcChannelConfig adc_config[3];

Svf lowPass;
Svf highPass;


GPIO fLED;
GPIO sLED;
GPIO tLED;
GPIO foLED;

GPIO* led[4] = {&fLED, &sLED, &tLED, &foLED};

GPIO firstHead;
GPIO secondHead;
GPIO thirdHead;
GPIO fourthHead;

GPIO* heads[4] = {&firstHead, &secondHead, &thirdHead, &fourthHead};


uint8_t activeHeadCount = 0;


DelayLine<float, MAX_DELAY> DSY_SDRAM_BSS sdramDelays[4];

bool activeHeads[4] = {false, false, false, true};
bool previousButtonsState[4] = {false, false, false, false};
float delayParameters[3];
float previousDelayParameters[3];
int position = 4;

float delay_out[4];
//float feedback[4];
float audio_out;
float sample_rate;

float delayTime[4];
int dupa = 0;


Delay delays[4];

float ProcessEmulation(float in)
{
  //filters.Process(in);
  return atan(in)*2/PI_F;
}


float Delay::ProcessDelay(float in, uint8_t i)
{
  fonepole(currentDelay, delayTarget, .001f);
  delay->SetDelay(sample_rate * currentDelay);
  float read = delay->Read();
  delay->Write(delayParameters[1] * read + in);
  return read;
};


void HandleDelays(float in, bool tapeHeadSetup[])
{
    activeHeadCount = 0;
    for(int i = 0; i < 4; i++) {
      if(tapeHeadSetup[i])
      {
        //led[i]->Write(true);
        delay_out[i] = delays[i].ProcessDelay(in, i);
        activeHeadCount++;
      }
      else
      {
        //led[i]->Write(false);
        delay_out[i] = 0.0f;
      }
    }
}


void AudioCallback(AudioHandle::InterleavingInputBuffer in,
                   AudioHandle::InterleavingOutputBuffer out,
                   size_t                    size)
{  
    loadMeter.OnBlockStart();
    HandlePeripherals(hw, delays, heads, delayParameters, previousDelayParameters, delayTime, activeHeads, previousButtonsState);

    for(size_t i = 0; i < size; i+=2)
      {
          audio_out = 0.0f;
          
          HandleDelays(in[LEFT], activeHeads);
          //filters.Process(in[LEFT]);
          

          if(activeHeadCount == 0) audio_out = in[LEFT];
          else
          {
            for(int j = 0; j < 4; j++) audio_out += delay_out[j] / (float)activeHeadCount;
            audio_out = (delayParameters[0] * audio_out) + ((1.0f - delayParameters[0]) * in[LEFT]);
          }
          
          out[LEFT] = audio_out;
          //out[RIGHT] = audio_out;
      }
    loadMeter.OnBlockEnd();
}



int main(void)
{
  hw.Init();
  
  hw.SetAudioBlockSize(4);
  hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_96KHZ);
  sample_rate = hw.AudioSampleRate();

  hw.StartLog();

  MyOledDisplay::Config disp_cfg;
  disp_cfg.driver_config.transport_config.i2c_address               = 0x3C;
  disp_cfg.driver_config.transport_config.i2c_config.periph         = I2CHandle::Config::Peripheral::I2C_1;
  disp_cfg.driver_config.transport_config.i2c_config.speed          = I2CHandle::Config::Speed::I2C_100KHZ;
  disp_cfg.driver_config.transport_config.i2c_config.mode           = I2CHandle::Config::Mode::I2C_MASTER;
  disp_cfg.driver_config.transport_config.i2c_config.pin_config.scl = {DSY_GPIOB, 8};    disp_cfg.driver_config.transport_config.i2c_config.pin_config.sda = {DSY_GPIOB, 9};
  display.Init(disp_cfg);

  System::Delay(500);

  highPass.Init(sample_rate);
  highPass.SetFreq(1000.0f);

  lowPass.Init(sample_rate);
  lowPass.SetFreq(2500.0f);

  //tap_tempo.Init(D12, GPIO::Mode::INPUT, GPIO::Pull::PULLUP);

  fLED.Init(D21, GPIO::Mode::OUTPUT);
  sLED.Init(D22, GPIO::Mode::OUTPUT);
  tLED.Init(D23, GPIO::Mode::OUTPUT);
  foLED.Init(D24, GPIO::Mode::OUTPUT);

  firstHead.Init(D10, GPIO::Mode::INPUT, GPIO::Pull::PULLUP);
  secondHead.Init(D9, GPIO::Mode::INPUT, GPIO::Pull::PULLUP);
  thirdHead.Init(D8, GPIO::Mode::INPUT, GPIO::Pull::PULLUP);
  fourthHead.Init(D7, GPIO::Mode::INPUT, GPIO::Pull::PULLUP);

  adc_config[0].InitSingle(A0); //    DRY / WET
  adc_config[1].InitSingle(A1); //    REPEATS
  adc_config[2].InitSingle(A2); //    DELAY TIME
  

  hw.adc.Init(adc_config, 3);

  hw.adc.Start();

  loadMeter.Init(hw.AudioSampleRate(), hw.AudioBlockSize());

  
  
  for(uint8_t i = 0; i < 4; i++)
  {
    sdramDelays[i].Init();
    delays[i].delay = &sdramDelays[i];
  }

  //System::Delay(5000);
  
  hw.StartAudio(AudioCallback);  

  

  for(;;){
      display.Fill(false);
      display.SetCursor(0, 0);
      char text[6];
      //display.WriteString(napis, Font_11x18, false);
      snprintf(text, sizeof(text), "%dms", (int)(delays[3].currentDelay * 1000.0f));
      
      display.WriteString(text, Font_16x26, true);
      display.Update();

      // get the current load (smoothed value and peak values)
        const float avgLoad = loadMeter.GetAvgCpuLoad();
        const float maxLoad = loadMeter.GetMaxCpuLoad();
        const float minLoad = loadMeter.GetMinCpuLoad();
        // print it to the serial connection (as percentages)
        hw.PrintLine("Processing Load %:");
        hw.PrintLine("Max: " FLT_FMT3, FLT_VAR3(maxLoad * 100.0f));
        hw.PrintLine("Avg: " FLT_FMT3, FLT_VAR3(avgLoad * 100.0f));
        hw.PrintLine("Min: " FLT_FMT3, FLT_VAR3(minLoad * 100.0f));
        // don't spam the serial connection too much
        System::Delay(500);
  }
}
