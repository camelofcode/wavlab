# wavlab #
wavlab is a command line utility for processing wave format audio files written in C++.
It has been made to re-familiarise myself with C++ after over a decade without using it. At the moment it is probably very production unready.

It currently supports PCM wave files, with or without bext/broadcast wave extensions at 8, 16, 24 and 32 bit depths.

Currently using C++14 and up and currently compiles in Visual Studio 2022 (tested - other C++14 compatible VS probably work). CMake version coming soon.
Only compiled and tested on windows so far but the aim from the start has been cross platform.

## Usage
run `wavlab [command]` using one of the following commands:

```
help
        description: shows the command list

eq-allpass <input> <frequency> <q> [output]
        description: applies an all pass filter to a wave file
        parameters:
                input: path to wave file
                frequency: centre frequency of the filter in Hz
                q: q factor of the filter
                output: path to output file. Optional, if not specified input will be overwritten

eq-bandpass <input> <frequency> <q> [output]
        description: applies a band pass filter to a wave file
        parameters:
                input: path to wave file
                frequency: centre frequency of the filter in Hz
                q: q factor of the filter
                output: path to output file. Optional, if not specified input will be overwritten

eq-highpass <input> <frequency> <q> [output]
        description: applies a high pass filter to a wave file
        parameters:
                input: path to wave file
                frequency: centre frequency of the filter in Hz
                q: q factor of the filter
                output: path to output file. Optional, if not specified input will be overwritten

eq-highshelf <input> <frequency> <gain> <q> [output]
        description: applies a positive or negative high shelf EQ to a wave file
        parameters:
                input: path to wave file
                frequency: frequency in Hz of the start of the shelf
                gain: gain in dB to cut/boost
                q: q factor of the filter
                output: path to output file. Optional, if not specified input will be overwritten

eq-lowpass <input> <frequency> <q> [output]
        description: applies a low pass filter to a wave file
        parameters:
                input: path to wave file
                frequency: centre frequency of the filter in Hz
                q: q factor of the filter
                output: path to output file. Optional, if not specified input will be overwritten

eq-lowshelf <input> <frequency> <gain> <q> [output]
        description: applies a positive or negative low shelf EQ to a wave file
        parameters:
                input: path to wave file
                frequency: frequency in Hz of the start of the shelf
                gain: gain in dB to cut/boost
                q: q factor of the filter
                output: path to output file. Optional, if not specified input will be overwritten

eq-peak <input> <frequency> <gain> <q> [output]
        description: applies a positive or negative peak EQ to a wave file
        parameters:
                input: path to wave file
                frequency: frequency in Hz of the cut/boost
                gain: gain in dB to cut/boost
                q: q factor of the filter
                output: path to output file. Optional, if not specified input will be overwritten

flip-phase <input> [output]
        description: reverses the phase of a wave file
        parameters:
                input: path to wave file
                output: path to output file. Optional, if not specified input will be overwritten

gain <input> <gain> [output]
        description: applies gain to a wave file
        parameters:
                input: path to wave file
                gain: gain multiplier
                output: path to output file. Optional, if not specified input will be overwritten

gain-db <input> <dB> [output]
        description: applies gain to a wave file, with input in decibels
        parameters:
                input: path to wave file
                dB: gain to apply in decibels
                output: path to output file. Optional, if not specified input will be overwritten

get-peak-db <input>
        description: shows the peak dB level of a wave file
        parameters:
                input: path to wave file

get-rms-db <input>
        description: shows the RMS dB level of a wave file
        parameters:
                input: path to wave file

join <output> <input...>
        description: joins multiple wave files into one.
        parameters:
                output: path to mixed wave file
                input: arbitrary number of input files (minimum 2)

left-right-to-stereo <leftInput> <rightInput> <output>
        description: converts two left/right mono wave files to a combined stereo wave file
        parameters:
                leftInput: path to left wave file
                rightInput: path to right wave file
                output: path to combined output file

mid-side-to-stereo <midInput> <sideInput> <output>
        description: converts two mid/side mono wave files to a combined stereo wave file
        parameters:
                midInput: path to mid wave file
                sideInput: path to side wave file
                output: path to combined output file

mix <output> <input...>
        description: mixes multiple wave files into one. Will attenuate and renormalise to ensure lack of clipping. Currently will start all files at zero.
        parameters:
                output: path to mixed wave file
                input: arbitrary number of input files (minimum 2)

normalise <input> [output]
        description: normalises a wave file
        parameters:
                input: path to wave file
                output: path to output file. Optional, if not specified input will be overwritten

reverse <input> [output]
        description: reverses a wave file
        parameters:
                input: path to wave file
                output: path to output file. Optional, if not specified input will be overwritten

saturate <input> <drive> [output]
        description: applies a tape saturation effect to a wave file
        parameters:
                input: path to wave file
                drive: increase for dirtier sound
                output: path to output file. Optional, if not specified input will be overwritten

sine-wave <output> <frequency> <sampleRate> <bitDepth> <length>
        description: writes a sine wave to the specified output file
        parameters:
                output: path to output wave file
                frequency: frequency in Hz of the sine wave
                sampleRate: sample rate of the output wave file
                bitDepth: bit depth of the output wave file
                length: length of the output file in milliseconds

split <input> <splitTimeMs...>
        description: splits a wave file at the specified times in milliseconds. Output files will be named {original}_1.wav, {original}_2.wav etc
        parameters:
                input: path to wave file to split
                splitTimeMs: arbitrary number of split times in milliseconds

split-mid-side <input>
        description: splits a stereo wave file to mid and side components, {original}_mid.wav and {original}_side.wav
        parameters:
                input: path to wave file

split-stereo <input>
        description: splits a stereo wave file to two mono files, {original}_l.wav and {original}_r.wav
        parameters:
                input: path to wave file

stat <input>
        description: show stats of a wave file
        parameters:
                input: path to wave file

swap-channels <input> [output]
        description: swaps the left and right channels of a stereo wave file
        parameters:
                input: path to wave file
                output: path to output file. Optional, if not specified input will be overwritten
```

## Upcoming features
* Generating square, sawtooth, triangle wave, white/pink noise
* Delay, reverb, chorus, phaser, flanger and many more FX
* Sample rate and bit rate (with dithering) conversion
* Fades and ADSR
* Dynamic processing (gate/compression)
* Adding broadcast chunks to existing waves
* Proper onscreen feedback for what is happening

## Known issues
* JUNK chunks are not written which may have as yet un-understood impact
* If chunks are in some weird random order that doesn't match spec bad stuff will happen

