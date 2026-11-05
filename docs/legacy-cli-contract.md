# Legacy C++ CLI contract

This is a frozen inventory of the command syntax registered at C++ revision
`1e7deca52766628fc3857c02a7730d91d66e6a49`. It exists to drive migration
documentation and compatibility tests; it is not a promise to retain every
historical semantic.

The baseline executable accepts `wavlab help` and `wavlab help <command>`.
With no command or an unknown command it writes an error and returns failure.
For processing commands with optional `[output]`, omitting the output
overwrites the input.

## Tools

```text
stat <input>
normalise <input> [output]
reverse <input> [output]
flip-phase <input> [output]
swap-channels <input> [output]
split-stereo <input>
left-right-to-stereo <leftInput> <rightInput> <output>
split-mid-side <input>
mid-side-to-stereo <midInput> <sideInput> <output>
get-peak-db <input>
get-rms-db <input>
gain <input> <gain> [output]
gain-db <input> <dB> [output]
mix <output> <input...>
split <input> <splitTimeMs...>
join <output> <input...>
```

Legacy split commands always derive filenames by replacing `.wav` with
`_l.wav`/`_r.wav`, `_mid.wav`/`_side.wav`, or numbered suffixes.

## Equalization

```text
eq-peak <input> <frequency> <gain> <q> [output]
eq-lowshelf <input> <frequency> <gain> <q> [output]
eq-highshelf <input> <frequency> <gain> <q> [output]
eq-allpass <input> <frequency> <q> [output]
eq-bandpass <input> <frequency> <q> [output]
eq-lowpass <input> <frequency> <q> [output]
eq-highpass <input> <frequency> <q> [output]
```

The legacy help calls the last parameter `q` for every filter, although the
peak, all-pass, and band-pass coefficient formula interprets it as bandwidth
in octaves.

## Effects

```text
saturate <input> <drive> [output]
digital-delay-mono <input> <delayMilliseconds> <feedback> <mix> [output] [maxOutputLengthMs]
digital-delay-stereo <input> <delayMillisecondsLeft> <feedbackLeft> <delayMillisecondsRight> <feedbackRight> <mix> [output] [maxOutputLengthMs]
analog-delay-mono <input> <delayMilliseconds> <feedback> <mix> [output] [maxOutputLengthMs]
analog-delay-stereo <input> <delayMillisecondsLeft> <feedbackLeft> <delayMillisecondsRight> <feedbackRight> <mix> [output] [maxOutputLengthMs]
tape-delay-mono <input> <delayMilliseconds> <feedback> <mix> [output] [maxOutputLengthMs]
tape-delay-stereo <input> <delayMillisecondsLeft> <feedbackLeft> <delayMillisecondsRight> <feedbackRight> <mix> [output] [maxOutputLengthMs]
reverb <input> <size> <time> <mix> [output] [maxOutputLengthMs]
```

Legacy effect semantics:

- delay time is an unsigned integer number of milliseconds;
- feedback and mix are percentages from 0 through 100;
- negative feedback is rejected;
- `maxOutputLengthMs` is an absolute total output length, not an additional
  tail, and zero defaults to the input duration;
- reverb size, time, and mix are percentages from 0 through 100; and
- saturation drive is passed through the undocumented linear transform
  `(drive + 0.7) * 1.2`, rather than being expressed in decibels.

## Generators

```text
sine-wave <output> <frequency> <sampleRate> <bitDepth> <length>
square-wave <output> <frequency> <sampleRate> <bitDepth> <length>
square-wave-additive <output> <frequency> <sampleRate> <bitDepth> <length> [numberOfHarmonics]
triangle-wave <output> <frequency> <sampleRate> <bitDepth> <length>
triangle-wave-additive <output> <frequency> <sampleRate> <bitDepth> <length> [numberOfHarmonics]
sawtooth-wave <output> <frequency> <sampleRate> <bitDepth> <length>
sawtooth-wave-additive <output> <frequency> <sampleRate> <bitDepth> <length> [numberOfHarmonics]
white-noise <output> <sampleRate> <bitDepth> <length>
pink-noise <output> <sampleRate> <bitDepth> <length>
```

`length` is documented in milliseconds. The baseline implementation performs
integer division by 1000 before multiplying by sample rate, which is recorded
as a defect rather than retained behavior.

## Planned migration

The compatibility policy is defined in
[ADR 0001](decisions/0001-cli-compatibility.md). Tests should use this file to
verify intentional aliases and release-note coverage, not to preserve known
bugs or unsafe parsing.
