#include <wavlab/Tools.h>
#include <wavlab/WavFile.h>

#include <exception>
#include <iostream>

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        std::cerr << "usage: wavlab_normalise_example <input.wav> <output.wav>\n";
        return 2;
    }

    try
    {
        auto wav = wavlab::WavFile::open(argv[1]);
        wavlab::Tools::normalise(wav);
        wav.save(argv[2]);
        return 0;
    }
    catch (const wavlab::WavError &error)
    {
        std::cerr << "could not process WAV file: " << error.what() << '\n';
        return 1;
    }
}
