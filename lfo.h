#include "math.h"

class LFO {
    public:
        LFO(float frequency, float sample_rate) : phase(0), increment(frequency / sample_rate) {}

        float Process() {
            float output = sinf(2 * PI_F * phase);
            phase += increment;
            if(phase >= 1.0f) phase -= 1.0f;
            return output;
        }

        void SetFrequency(float frequency, float sample_rate) {
            increment = frequency / sample_rate;
        }

    private:
        float phase;
        float increment;
};