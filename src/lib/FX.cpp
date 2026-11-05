#include <wavlab/FX.h>
#include "Constants.h"
#include <wavlab/Delay.h>
#include <wavlab/Tools.h>
#include <wavlab/EQ.h>
#include <cmath>

namespace wavlab
{

/* saturate command */
void FX::saturate(WavFile& wave, double drive)
{
	std::vector<double> newSamples;

	double gainFactor = (drive + 0.7) * 1.2;

	for (auto& sample : wave.getSamples())
	{
		double saturated = saturateSample(sample, gainFactor);
		newSamples.push_back(saturated);
	}
	wave.setSamples(newSamples);
}

double FX::saturateSample(double in, double gainFactor)
{
	//At one point (2009) this algorithm existed at http://tap-plugins.sourceforge.net/ladspa/sigmoid.html
	double preSaturation = in * gainFactor;
	return 2 / (1 + (pow(Constants::E, (-5.0 * preSaturation)))) - 1;
}

/* digital-delay-mono command*/
void FX::delay(WavFile& wave, uint32_t delayMs, double feedback, double mix, uint32_t maxFileLengthMs)
{
	//Passes to mono overload with passthrough feedback processor.
	delay(wave, delayMs, feedback, mix, maxFileLengthMs, [](double in) { return in; });
}

void FX::delay(WavFile& wave, uint32_t delayMsL, double feedbackL, uint32_t delayMsR, double feedbackR, double mix, uint32_t maxFileLengthMs)
{
	//Passes to stereo overload with passthrough feedback processor.
	delay(wave, true, delayMsL, feedbackL, delayMsR, feedbackR, mix, maxFileLengthMs, [](double in) { return in; });
}

void FX::delay(WavFile& wave, uint32_t delayMs, double feedback, double mix, uint32_t maxFileLengthMs, const std::function<double(double input)>& feedbackProcessor)
{
	//Passes to stereo overload with specified feedback processor.
	delay(wave, false, delayMs, feedback, delayMs, feedback, mix, maxFileLengthMs, feedbackProcessor);
}

/* full delay implementation that will work for all possible types */

void FX::delay(WavFile& wave, bool stereo, uint32_t delayMsL, double feedbackL, uint32_t delayMsR, double feedbackR, double mix, uint32_t maxFileLengthMs, const std::function<double(double input)>& feedbackProcessor)
{
	double scaledFeedbackL = feedbackL / 100.0;
	double scaledFeedbackR = feedbackR / 100.0;
	double scaledMix = mix / 100.0;
	std::vector<double> originalSamples = wave.getSamples();
	auto maxNumberOfSamples = static_cast<uint32_t>(maxFileLengthMs == 0 ? (originalSamples.size() / wave.getNumberOfChannels()) : wave.getSampleRate() * (maxFileLengthMs / 1000.0)); // NOLINT(cppcoreguidelines-narrowing-conversions,bugprone-integer-division)
	uint32_t currentNumberOfSamples = 0;
	double maxSample = 0.0;

	std::vector<double> newSamples;

	if (wave.getNumberOfChannels() == 1)
	{
		if (stereo)
		{
			Delay delayL(delayMsL * 2, delayMsL, wave.getSampleRate());
			Delay delayR(delayMsR * 2, delayMsR, wave.getSampleRate());

			for (auto& sample : originalSamples)
			{
				if (sample > maxSample)
				{
					maxSample = sample;
				}
				newSamples.push_back(delayL.next(sample, scaledFeedbackL, feedbackProcessor, scaledMix));
				newSamples.push_back(delayR.next(sample, scaledFeedbackR, feedbackProcessor, scaledMix));
				currentNumberOfSamples++;
			}
			//Lengthen file for delay tail, up to max value provided
			while (currentNumberOfSamples < maxNumberOfSamples)
			{
				newSamples.push_back(delayL.next(0, scaledFeedbackL, feedbackProcessor, scaledMix));
				newSamples.push_back(delayR.next(0, scaledFeedbackR, feedbackProcessor, scaledMix));
				currentNumberOfSamples++;
			}
		}
		else
		{
			Delay delay(delayMsL * 2, delayMsL, wave.getSampleRate());
			for (auto& sample : originalSamples)
			{
				if (sample > maxSample)
				{
					maxSample = sample;
				}
				newSamples.push_back(delay.next(sample, scaledFeedbackL, feedbackProcessor, scaledMix));
				currentNumberOfSamples++;
			}
			//Lengthen file for delay tail, up to max value provided
			while (currentNumberOfSamples < maxNumberOfSamples)
			{
				newSamples.push_back(delay.next(0, scaledFeedbackL, feedbackProcessor, scaledMix));
				currentNumberOfSamples++;
			}
		}

	}
	else if (wave.getNumberOfChannels() == 2)
	{
		Delay delayL(delayMsL * 2, delayMsL, wave.getSampleRate());
		Delay delayR(delayMsR * 2, delayMsR, wave.getSampleRate());

		for (size_t i = 0; i < originalSamples.size() - 1; i = i + 2)
		{
			double left = originalSamples[i];
			double right = originalSamples[i + 1];

			if (left > maxSample)
			{
				maxSample = left;
			}
			if (right > maxSample)
			{
				maxSample = right;
			}

			newSamples.push_back(delayL.next(left, scaledFeedbackL, feedbackProcessor, scaledMix));
			newSamples.push_back(delayR.next(right, scaledFeedbackR, feedbackProcessor, scaledMix));
			currentNumberOfSamples++;
		}
		//Lengthen sample for delay tail, up to max value provided
		while (currentNumberOfSamples < maxNumberOfSamples)
		{
			newSamples.push_back(delayL.next(0, scaledFeedbackL, feedbackProcessor, scaledMix));
			newSamples.push_back(delayR.next(0, scaledFeedbackR, feedbackProcessor, scaledMix));
			currentNumberOfSamples++;
		}
	}
	//Makes sure a mono input file becomes stereo if stereo delay applied
	wave.setNumberOfChannels(stereo || wave.getNumberOfChannels() == 2 ? 2 : 1);

	wave.setSamples(newSamples);

	//Ensures input and output are the same level when mix = 50%
	Tools::normalise(wave, maxSample);
}

/* reverb command */
void FX::reverb(WavFile& wave, double size, double time, double mix, uint32_t maxFileLengthMs)
{
	//I think I got this algorithm from a lecture slide but I can't find it anywhere and it would have been 13 years ago, so apologies for not accrediting correctly.

	double scaledSize = 2.0 - ((size / 100.0) + 0.5);
	double scaledTime = time / 100.0;
	double scaledMix = mix / 100.0;
	std::vector<double> originalSamples = wave.getSamples();
	std::vector<double> newSamples;

	double feedback1 = pow(0.001, ((28.7 * scaledSize) / (scaledTime * 3000.0)));
	double feedback2 = pow(0.001, ((37.1 * scaledSize) / (scaledTime * 3000.0)));
	double feedback3 = pow(0.001, ((41.1 * scaledSize) / (scaledTime * 3000.0)));
	double feedback4 = pow(0.001, ((43.7 * scaledSize) / (scaledTime * 3000.0)));

	Delay lateReflections1(100, 97, wave.getSampleRate());
	Delay lateReflections2(100, 33, wave.getSampleRate());

	auto maxNumberOfSamples = static_cast<uint32_t>(maxFileLengthMs == 0 ? (originalSamples.size() / wave.getNumberOfChannels()) : wave.getSampleRate() * (maxFileLengthMs / 1000.0)); // NOLINT(cppcoreguidelines-narrowing-conversions,bugprone-integer-division)
	uint32_t currentNumberOfSamples = 0;
	double maxSample = 0.0;

	auto processSample = [&lateReflections1, &lateReflections2, &feedback1, &feedback2, &feedback3, &feedback4, &scaledMix](double sample, Delay& combFilter1, Delay& combFilter2, Delay& combFilter3, Delay& combFilter4)
	{
		double reverb = (combFilter1.next(sample, feedback1) + combFilter2.next(sample, feedback2) + combFilter3.next(sample, feedback3) + combFilter4.next(sample, feedback4)) / 4.0;
		reverb = lateReflections1.next(reverb, 0.7);
		reverb = lateReflections2.next(reverb, 0.7);
		return ((1.0 - scaledMix) * sample) + (scaledMix * reverb);
	};

	if (wave.getNumberOfChannels() == 1)
	{
		Delay combFilter1(100, 15, wave.getSampleRate());
		Delay combFilter2(100, 18, wave.getSampleRate());
		Delay combFilter3(100, 20, wave.getSampleRate());
		Delay combFilter4(100, 22, wave.getSampleRate());

		for (auto& sample : originalSamples)
		{
			if (sample > maxSample)
			{
				maxSample = sample;
			}
			newSamples.push_back(processSample(sample, combFilter1, combFilter2, combFilter3, combFilter4));
			currentNumberOfSamples++;
		}
		//Lengthen sample for delay tail, up to max value provided
		while (currentNumberOfSamples < maxNumberOfSamples)
		{
			newSamples.push_back(processSample(0, combFilter1, combFilter2, combFilter3, combFilter4));
			currentNumberOfSamples++;
		}
	}
	else if (wave.getNumberOfChannels() == 2)
	{
		Delay combFilter1L(100, 15, wave.getSampleRate());
		Delay combFilter2L(100, 18, wave.getSampleRate());
		Delay combFilter3L(100, 20, wave.getSampleRate());
		Delay combFilter4L(100, 22, wave.getSampleRate());

		Delay combFilter1R(100, 15, wave.getSampleRate());
		Delay combFilter2R(100, 18, wave.getSampleRate());
		Delay combFilter3R(100, 20, wave.getSampleRate());
		Delay combFilter4R(100, 22, wave.getSampleRate());

		for (size_t i = 0; i < originalSamples.size() - 1; i = i + 2)
		{
			double left = originalSamples[i];
			double right = originalSamples[i + 1];

			if (left > maxSample)
			{
				maxSample = left;
			}
			if (right > maxSample)
			{
				maxSample = right;
			}

			newSamples.push_back(processSample(left, combFilter1L, combFilter2L, combFilter3L, combFilter4L));
			newSamples.push_back(processSample(right, combFilter1R, combFilter2R, combFilter3R, combFilter4R));
			currentNumberOfSamples++;
		}
		//Lengthen sample for delay tail, up to max value provided
		while (currentNumberOfSamples < maxNumberOfSamples)
		{
			newSamples.push_back(processSample(0, combFilter1L, combFilter2L, combFilter3L, combFilter4L));
			newSamples.push_back(processSample(0, combFilter1R, combFilter2R, combFilter3R, combFilter4R));
			currentNumberOfSamples++;
		}
	}

	wave.setSamples(newSamples);

	//Ensures input and output are the same level when mix = 50%
	Tools::normalise(wave, maxSample);
}

}
