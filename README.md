# wavlab #
wavlab is a command line utility for processing wave format audio files written in C++.
It has been made to re-familiarise myself with C++ after over a decade without using it. At the moment it is probably very production unready.

It currently supports PCM wave files, with or without bext/broadcast wave extensions at 16, 24 and 32 bit depths.

What C++ versions is it compatible with? I don't know. 20 for sure, 17 almost certainly, 14 and 11 probably.

It currently compiles in Visual Studio 2022. CMake version coming soon.
Only compiled and tested on windows so far but the aim from the start has been cross platform.

## Usage
```
wavlab help
        description: shows the command list

wavlab stat <input>
        description: show stats of a wave file
        parameters:
                input: path to wave file

wavlab normalise <input> [output]
        description: normalises a wave file
        parameters:
                input: path to wave file
                output: path to output file. Optional, if not specified input will be overwritten

wavlab reverse <input> [output]
        description: reverses a wave file
        parameters:
                input: path to wave file
                output: path to output file. Optional, if not specified input will be overwritten

wavlab flip-phase <input> [output]
        description: reverses the phase of a wave file
        parameters:
                input: path to wave file
                output: path to output file. Optional, if not specified input will be overwritten

wavlab swap-channels <input> [output]
        description: swaps the left and right channels of a stereo wave file
        parameters:
                input: path to wave file
                output: path to output file. Optional, if not specified input will be overwritten

wavlab get-peak-db <input>
        description: shows the peak dB level of a wave file
        parameters:
                input: path to wave file

wavlab gain <input> <gain> [output]
        description: applies gain to a wave file
        parameters:
                input: path to wave file
                gain: gain multiplier
                output: path to output file. Optional, if not specified input will be overwritten

wavlab gain-db <input> <dB> [output]
        description: applies gain to a wave file, with input in decibels
        parameters:
                input: path to wave file
                dB: gain to apply in decibels
                output: path to output file. Optional, if not specified input will be overwritten
				
wavlab eq-peak <input> <frequency> <gain> <q> [output]
        description: applies a positive or negative peak EQ to a wave file
        parameters:
                input: path to wave file
                frequency: frequency in Hz of the cut/boost
                gain: gain in dB to cut/boost
                q: q factor of the filter
                output: path to output file. Optional, if not specified input will be overwritten

wavlab eq-lowshelf <input> <frequency> <gain> <q> [output]
        description: applies a positive or negative low shelf EQ to a wave file
        parameters:
                input: path to wave file
                frequency: frequency in Hz of the start of the shelf
                gain: gain in dB to cut/boost
                q: q factor of the filter
                output: path to output file. Optional, if not specified input will be overwritten

wavlab eq-highshelf <input> <frequency> <gain> <q> [output]
        description: applies a positive or negative high shelf EQ to a wave file
        parameters:
                input: path to wave file
                frequency: frequency in Hz of the start of the shelf
                gain: gain in dB to cut/boost
                q: q factor of the filter
                output: path to output file. Optional, if not specified input will be overwritten

wavlab eq-allpass <input> <frequency> <q> [output]
        description: applies an all pass filter to a wave file
        parameters:
                input: path to wave file
                frequency: centre frequency of the filter in Hz
                q: q factor of the filter
                output: path to output file. Optional, if not specified input will be overwritten

wavlab eq-bandpass <input> <frequency> <q> [output]
        description: applies a band pass filter to a wave file
        parameters:
                input: path to wave file
                frequency: centre frequency of the filter in Hz
                q: q factor of the filter
                output: path to output file. Optional, if not specified input will be overwritten

wavlab eq-lowpass <input> <frequency> <q> [output]
        description: applies a low pass filter to a wave file
        parameters:
                input: path to wave file
                frequency: centre frequency of the filter in Hz
                q: q factor of the filter
                output: path to output file. Optional, if not specified input will be overwritten

wavlab eq-highpass <input> <frequency> <q> [output]
        description: applies a high pass filter to a wave file
        parameters:
                input: path to wave file
                frequency: centre frequency of the filter in Hz
                q: q factor of the filter
                output: path to output file. Optional, if not specified input will be overwritten

wavlab saturate <input> <drive> [output]
        description: applies a tape saturation effect to a wave file
        parameters:
                input: path to wave file
                drive: increase for dirtier sound
                output: path to output file. Optional, if not specified input will be overwritten

wavlab sine-wave <output> <frequency> <sampleRate> <bitDepth> <length>
        description: writes a sine wave to the specified output file
        parameters:
                output: path to output wave file
                frequency: frequency in Hz of the sine wave
                sampleRate: sample rate of the output wave file
                bitDepth: bit depth of the output wave file
                length: length of the output file in milliseconds
```

## Upcoming features
* Generating square, sawtooth, triangle wave, white/pink noise
* Mixing wave files
* Joining and splitting wave files
* Delay, reverb, chorus, phaser, flanger and many more FX
* Sample rate and bit rate (with dithering) conversion
* Fades and ADSR
* Dynamic processing (gate/compression)
* Adding broadcast chunks to existing waves
* 8 bit support for extra crunchyness

## Known issues
* JUNK chunks are not written which may have as yet un-understood impact
* If chunks are in some weird random order that doesn't match spec bad stuff will happen

