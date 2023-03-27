#include "wavestream.h"

using namespace wave;

const string desktop = "C:\\Users\\olsud\\Desktop\\";

int main()
{
    wavestream wave;
    wave.frequencyWithWav(196/2, 0xffff, 5, desktop + "g196hz viola.wav");
    wave.saveAs(desktop + "viola.wav");

    std::cout << "\n\ndone\n";
    return 0;
}
