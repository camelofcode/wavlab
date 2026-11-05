#include "BinaryHelpers.h"
#include "ConversionHelpers.h"
#include <fstream>
#include <limits>
#include <sstream>
#include <string>
#include <utility>
#include <vector>
#include <wavlab/WavFile.h>

namespace wavlab
{

namespace
{

[[nodiscard]] std::uint16_t bytesPerSample(std::uint16_t bits)
{
    switch (bits)
    {
    case 8:
        return 1;
    case 16:
        return 2;
    case 24:
        return 3;
    case 32:
        return 4;
    default:
        throw WavError(WavErrorCode::unsupportedBitsPerSample,
                       "unsupported PCM bit depth " + std::to_string(bits) +
                           "; expected 8, 16, 24, or 32 bits");
    }
}

} // namespace

WavError::WavError(WavErrorCode code, std::string message)
    : std::runtime_error(std::move(message)), code_(code)
{
}

WavErrorCode WavError::code() const noexcept { return code_; }

WavFile WavFile::open(const std::filesystem::path &path) { return WavFile(path, true); }

WavFile WavFile::openMetadata(const std::filesystem::path &path) { return WavFile(path, false); }

WavFile WavFile::create(std::uint16_t numberOfChannels, std::uint32_t sampleRate,
                        std::uint16_t bitsPerSample)
{
    return WavFile(numberOfChannels, sampleRate, bitsPerSample);
}

WavFile::WavFile(const std::filesystem::path &path, bool loadSamples)
{
    std::ifstream infile(path, std::ifstream::binary);

    if (!infile)
    {
        throw WavError(WavErrorCode::io, "could not open WAV file for reading: " + path.string());
    }

    infile.seekg(0, std::ios::end);
    lengthInBytes = infile.tellg();
    infile.seekg(0, std::ios::beg);

    fileChunkId = BinaryHelpers::readString(infile, 4);
    fileChunkSize = BinaryHelpers::readInt32(infile);
    format = BinaryHelpers::readString(infile, 4);

    formatChunkId = BinaryHelpers::readString(infile, 4);
    formatChunkSize = BinaryHelpers::readInt32(infile);
    audioFormat = BinaryHelpers::readInt16(infile);
    numberOfChannels_ = BinaryHelpers::readInt16(infile);
    sampleRate_ = BinaryHelpers::readInt32(infile);
    byteRate = BinaryHelpers::readInt32(infile);
    blockAlign = BinaryHelpers::readInt16(infile);
    bitsPerSample_ = BinaryHelpers::readInt16(infile);

    auto nextChunk = BinaryHelpers::readString(infile, 4);

    if (nextChunk == "bext")
    {
        isBroadcastWav = true;
        readBextFields(infile, false);

        nextChunk = BinaryHelpers::readString(infile, 4);
    }

    if (nextChunk == "JUNK" || nextChunk == "junk") // I've seen both........
    {
        auto junkSize = BinaryHelpers::readInt32(infile);
        if (junkSize % 2 != 0)
        {
            junkSize += 1;
        }
        infile.ignore(junkSize);

        nextChunk = BinaryHelpers::readString(
            infile, 4); // Should be "data" if we have read the bext chunk and junk chunks properly.
    }

    if (nextChunk == "data")
    {
        dataChunkId = nextChunk;
        dataChunkSize = BinaryHelpers::readInt32(infile);
        dataChunkStart = infile.tellg();
    }

    if ((dataChunkStart + dataChunkSize) != static_cast<int64_t>(lengthInBytes))
    {
        // There is a footer...
        infile.seekg(dataChunkStart + dataChunkSize + 1);
        nextChunk = BinaryHelpers::readString(infile, 4);
        if (nextChunk == "bext")
        {
            // Unsurprisingly some geniuses think it makes sense to put BWAV ***HEADER*** info as a
            // footer. Also, unsurprisingly those fine folks at AVID seem to think this is a great
            // idea. So I'm supporting it I guess.
            isBroadcastWav = true;
            readBextFields(infile, true);
        }
        else
        {
            // It's a non-standard footer. Let's just ignore it. This branch exists to note I'm
            // doing it on purpose, I'm sure it will be optimised out.
        }
    }

    validate();
    numberOfSamples = dataChunkSize / bytesPerSample(bitsPerSample_);
    samplesLoaded = loadSamples;

    if (loadSamples)
    {
        infile.seekg(dataChunkStart);

        samples_.resize(numberOfSamples);

        if (bitsPerSample_ == 8)
        {
            for (size_t i = 0; i < numberOfSamples; i++)
            {
                samples_[i] = ConversionHelpers::eightBitToDouble(BinaryHelpers::readInt8(infile));
            }
        }
        if (bitsPerSample_ == 16)
        {
            for (size_t i = 0; i < numberOfSamples; i++)
            {
                samples_[i] =
                    ConversionHelpers::sixteenBitToDouble(BinaryHelpers::readInt16(infile));
            }
        }
        else if (bitsPerSample_ == 24)
        {
            for (size_t i = 0; i < numberOfSamples; i++)
            {
                samples_[i] =
                    ConversionHelpers::twentyFourBitToDouble(BinaryHelpers::readInt24(infile));
            }
        }
        else if (bitsPerSample_ == 32)
        {
            for (size_t i = 0; i < numberOfSamples; i++)
            {
                samples_[i] =
                    ConversionHelpers::thirtyTwoBitToDouble(BinaryHelpers::readInt32(infile));
            }
        }
    }
}

WavFile::WavFile(std::uint16_t numberOfChannels, std::uint32_t sampleRate,
                 std::uint16_t bitsPerSample)
    : fileChunkId("RIFF"), fileChunkSize(36), format("WAVE"), formatChunkId("fmt "),
      formatChunkSize(16), audioFormat(1), numberOfChannels_(numberOfChannels),
      sampleRate_(sampleRate), bitsPerSample_(bitsPerSample), dataChunkId("data"),
      dataChunkStart(44), samplesLoaded(true)
{
    setDefaultBextFields();
    updatePcmLayout();
    lengthInBytes = 44;
}

void WavFile::validate() const
{
    if (fileChunkId != "RIFF")
    {
        throw WavError(WavErrorCode::invalidFile,
                       "invalid WAV file: expected RIFF identifier, found " + fileChunkId);
    }

    if (fileChunkSize != lengthInBytes - 8)
    {
        throw WavError(WavErrorCode::invalidFile, "invalid WAV file: RIFF header declares " +
                                                      std::to_string(fileChunkSize + 8ULL) +
                                                      " bytes, but the file contains " +
                                                      std::to_string(lengthInBytes));
    }

    if (format != "WAVE")
    {
        throw WavError(WavErrorCode::invalidFile,
                       "invalid WAV file: expected WAVE format, found " + format);
    }

    if (formatChunkId != "fmt ")
    {
        throw WavError(WavErrorCode::invalidFile,
                       "invalid WAV file: expected fmt chunk, found " + formatChunkId);
    }

    if (audioFormat != 1)
    {
        throw WavError(WavErrorCode::invalidFile, "invalid WAV file: audio format " +
                                                      std::to_string(audioFormat) + " is not PCM");
    }

    if (numberOfChannels_ != 1 && numberOfChannels_ != 2)
    {
        throw WavError(WavErrorCode::unsupportedChannelCount,
                       "unsupported channel count " + std::to_string(numberOfChannels_) +
                           "; only mono and stereo PCM are supported");
    }

    if (sampleRate_ == 0)
    {
        throw WavError(WavErrorCode::invalidSampleRate,
                       "invalid sample rate 0; it must be greater than zero");
    }

    const auto expectedBlockAlign =
        static_cast<std::uint16_t>(numberOfChannels_ * bytesPerSample(bitsPerSample_));
    if (blockAlign != expectedBlockAlign)
    {
        throw WavError(WavErrorCode::invalidFile, "invalid WAV file: block alignment is " +
                                                      std::to_string(blockAlign) + ", expected " +
                                                      std::to_string(expectedBlockAlign));
    }

    const auto expectedByteRate = static_cast<std::uint64_t>(sampleRate_) * expectedBlockAlign;
    if (expectedByteRate > std::numeric_limits<std::uint32_t>::max())
    {
        throw WavError(WavErrorCode::sizeOverflow,
                       "byte rate exceeds the WAV limit for sample rate " +
                           std::to_string(sampleRate_) + " and block alignment " +
                           std::to_string(expectedBlockAlign));
    }
    if (byteRate != expectedByteRate)
    {
        throw WavError(WavErrorCode::invalidFile, "invalid WAV file: byte rate is " +
                                                      std::to_string(byteRate) + ", expected " +
                                                      std::to_string(expectedByteRate));
    }
    if (dataChunkId != "data")
    {
        throw WavError(WavErrorCode::invalidFile, "invalid WAV file: missing data chunk");
    }
    if (dataChunkSize % expectedBlockAlign != 0)
    {
        throw WavError(WavErrorCode::invalidFile,
                       "invalid WAV file: data chunk size " + std::to_string(dataChunkSize) +
                           " does not contain complete " + std::to_string(expectedBlockAlign) +
                           "-byte sample frames");
    }
}

std::string WavFile::details() const
{
    std::ostringstream report;
    report << "Number Of Channels: " << numberOfChannels_ << '\n'
           << "Sample Rate: " << sampleRate_ << '\n'
           << "Bit Depth: " << bitsPerSample_;

    if (isBroadcastWav)
    {
        report << '\n'
               << "Description: " << description << '\n'
               << "Originator: " << originator << '\n'
               << "Originator Reference: " << originatorReference << '\n'
               << "Origination Date/Time: " << originationDate << ' ' << originationTime << '\n'
               << "Broadcast Wave Version: " << version;
    }

    return report.str();
}

std::string WavFile::debugDetails() const
{
    std::ostringstream report;
    report << "lengthInBytes: " << lengthInBytes << '\n'
           << "fileChunkId: " << fileChunkId << '\n'
           << "fileChunkSize: " << fileChunkSize << '\n'
           << "format: " << format << '\n'
           << "formatChunkId: " << formatChunkId << '\n'
           << "formatChunkSize: " << formatChunkSize << '\n'
           << "audioFormat: " << audioFormat << '\n'
           << "numberOfChannels: " << numberOfChannels_ << '\n'
           << "sampleRate: " << sampleRate_ << '\n'
           << "byteRate: " << byteRate << '\n'
           << "blockAlign: " << blockAlign << '\n'
           << "bitsPerSample: " << bitsPerSample_ << '\n'
           << "dataChunkId: " << dataChunkId << '\n'
           << "dataChunkSize: " << dataChunkSize;

    if (isBroadcastWav)
    {
        report << '\n'
               << "bextChunkId: " << bextChunkId << '\n'
               << "bextChunkSize: " << bextChunkSize << '\n'
               << "description: " << description << '\n'
               << "originator: " << originator << '\n'
               << "originatorReference: " << originatorReference << '\n'
               << "originationDate: " << originationDate << '\n'
               << "originationTime: " << originationTime << '\n'
               << "timeReferenceLow: " << timeReferenceLow << '\n'
               << "timeReferenceHigh: " << timeReferenceHigh << '\n'
               << "version: " << version;
    }

    return report.str();
}

void WavFile::save(const std::filesystem::path &path)
{
    if (!samplesLoaded)
    {
        throw WavError(WavErrorCode::samplesNotLoaded, "sample data was not loaded");
    }
    if (samples_.size() % numberOfChannels_ != 0)
    {
        throw WavError(WavErrorCode::incompleteSampleFrame,
                       std::to_string(samples_.size()) +
                           " samples do not form complete frames for " +
                           std::to_string(numberOfChannels_) + " channels");
    }

    const auto dataSize = samples_.size() * bytesPerSample(bitsPerSample_);
    if (dataSize > std::numeric_limits<std::uint32_t>::max())
    {
        throw WavError(WavErrorCode::sizeOverflow, "sample data is too large for a WAV file");
    }

    formatChunkSize = 16;
    dataChunkSize = static_cast<std::uint32_t>(dataSize);
    bextChunkSize = isBroadcastWav ? 602 : 0;
    const std::uint64_t fileSize =
        36ULL + dataChunkSize + (isBroadcastWav ? 8ULL + bextChunkSize : 0ULL);
    if (fileSize > std::numeric_limits<std::uint32_t>::max())
    {
        throw WavError(WavErrorCode::sizeOverflow, "RIFF data is too large for a WAV file");
    }
    fileChunkSize = static_cast<std::uint32_t>(fileSize);
    lengthInBytes = fileChunkSize + 8ULL;

    validate();

    std::ofstream outfile(path, std::ofstream::binary);

    if (!outfile)
    {
        throw WavError(WavErrorCode::io, "could not open WAV file for writing: " + path.string());
    }

    BinaryHelpers::writeString(outfile, fileChunkId);
    BinaryHelpers::writeInt32(outfile, fileChunkSize);
    BinaryHelpers::writeString(outfile, format);

    BinaryHelpers::writeString(outfile, formatChunkId);
    BinaryHelpers::writeInt32(outfile, formatChunkSize);
    BinaryHelpers::writeInt16(outfile, audioFormat);
    BinaryHelpers::writeInt16(outfile, numberOfChannels_);
    BinaryHelpers::writeInt32(outfile, sampleRate_);
    BinaryHelpers::writeInt32(outfile, byteRate);
    BinaryHelpers::writeInt16(outfile, blockAlign);
    BinaryHelpers::writeInt16(outfile, bitsPerSample_);

    if (isBroadcastWav)
    {
        BinaryHelpers::writeString(outfile, bextChunkId);
        BinaryHelpers::writeInt32(outfile, bextChunkSize);

        BinaryHelpers::writeString(outfile, description, 256);
        BinaryHelpers::writeString(outfile, originator, 32);
        BinaryHelpers::writeString(outfile, originatorReference, 32);
        BinaryHelpers::writeString(outfile, originationDate, 10);
        BinaryHelpers::writeString(outfile, originationTime, 8);

        BinaryHelpers::writeInt32(outfile, timeReferenceLow);
        BinaryHelpers::writeInt32(outfile, timeReferenceHigh);
        BinaryHelpers::writeInt16(outfile, version);

        // Write reserved bytes. There should be 254 because we are not yet supporting v1 and v2
        // fields.
        for (size_t i = 0; i < 254; i++)
        {
            outfile.write("\0", 1);
        }
    }

    // TODO: Write Junk chunk?! Not sure why we need it
    //"To align RIFF chunks to certain boundaries (i.e. 2048bytes for CD-ROMs) the RIFF
    // specification includes a JUNK chunk. Its contents are to be skipped when reading. When
    // writing RIFFs, JUNK chunks should not have odd number as Size."

    BinaryHelpers::writeString(outfile, dataChunkId);
    BinaryHelpers::writeInt32(outfile, dataChunkSize);

    if (bitsPerSample_ == 8)
    {
        for (size_t i = 0; i < numberOfSamples; i++)
        {
            BinaryHelpers::writeInt8(outfile, ConversionHelpers::doubleToEightBit(samples_[i]));
        }
    }
    if (bitsPerSample_ == 16)
    {
        for (size_t i = 0; i < numberOfSamples; i++)
        {
            BinaryHelpers::writeInt16(outfile, ConversionHelpers::doubleToSixteenBit(samples_[i]));
        }
    }
    else if (bitsPerSample_ == 24)
    {
        for (size_t i = 0; i < numberOfSamples; i++)
        {
            BinaryHelpers::writeInt24(outfile,
                                      ConversionHelpers::doubleToTwentyFourBit(samples_[i]));
        }
    }
    else if (bitsPerSample_ == 32)
    {
        for (size_t i = 0; i < numberOfSamples; i++)
        {
            BinaryHelpers::writeInt32(outfile,
                                      ConversionHelpers::doubleToThirtyTwoBit(samples_[i]));
        }
    }

    outfile.close();
    if (!outfile)
    {
        throw WavError(WavErrorCode::io, "failed while writing WAV file: " + path.string());
    }
}

std::uint16_t WavFile::numberOfChannels() const noexcept { return numberOfChannels_; }

std::uint32_t WavFile::sampleRate() const noexcept { return sampleRate_; }

std::uint16_t WavFile::bitsPerSample() const noexcept { return bitsPerSample_; }

std::size_t WavFile::sampleCount() const noexcept { return numberOfSamples; }

std::size_t WavFile::frameCount() const noexcept
{
    return numberOfChannels_ == 0 ? 0 : numberOfSamples / numberOfChannels_;
}

bool WavFile::hasLoadedSamples() const noexcept { return samplesLoaded; }

std::span<const double> WavFile::samples() const

{
    if (!samplesLoaded)
    {
        throw WavError(WavErrorCode::samplesNotLoaded, "sample data was not loaded");
    }
    return samples_;
}

std::span<double> WavFile::samples()
{
    if (!samplesLoaded)
    {
        throw WavError(WavErrorCode::samplesNotLoaded, "sample data was not loaded");
    }
    return samples_;
}

void WavFile::setNumberOfChannels(std::uint16_t number)
{
    if (number != 1 && number != 2)
    {
        throw WavError(WavErrorCode::unsupportedChannelCount,
                       "unsupported channel count " + std::to_string(number) +
                           "; only mono and stereo PCM are supported");
    }
    if (numberOfSamples % number != 0)
    {
        throw WavError(WavErrorCode::incompleteSampleFrame,
                       std::to_string(numberOfSamples) +
                           " samples do not form complete frames for " + std::to_string(number) +
                           " channels");
    }

    numberOfChannels_ = number;
    updatePcmLayout();
}

void WavFile::setSamples(std::vector<double> newSamples)
{
    if (newSamples.size() % numberOfChannels_ != 0)
    {
        throw WavError(WavErrorCode::incompleteSampleFrame,
                       std::to_string(newSamples.size()) +
                           " samples do not form complete frames for " +
                           std::to_string(numberOfChannels_) + " channels");
    }

    samples_ = std::move(newSamples);
    numberOfSamples = samples_.size();
    samplesLoaded = true;
}

// Private functions
void WavFile::readBextFields(std::ifstream &file, bool isFooter)
{
    bextChunkId = "bext";
    bextChunkSize = BinaryHelpers::readInt32(file);

    description = BinaryHelpers::readString(file, 256);
    originator = BinaryHelpers::readString(file, 32);
    originatorReference = BinaryHelpers::readString(file, 32);
    originationDate = BinaryHelpers::readString(file, 10);
    originationTime = BinaryHelpers::readString(file, 8);
    timeReferenceLow = BinaryHelpers::readInt32(file);
    timeReferenceHigh = BinaryHelpers::readInt32(file);
    version = BinaryHelpers::readInt16(file);

    if (isFooter)
    {
        // Might be a JUNK Chunk, but we are pretty much done caring at this point.
    }
    else
    {
        // Need to get to end of this chunk. We are ignoring v1 and v2 fields for now so need to
        // skip the 254 reserved bytes
        std::streamoff position = file.tellg();
        file.seekg(position + 254);
    }
}

void WavFile::setDefaultBextFields()
{
    isBroadcastWav = false;
    bextChunkId = "bext";
    description = "";
    originator = "";
    originatorReference = "";
    originationDate = "";
    originationTime = "";
    timeReferenceLow = 0;
    timeReferenceHigh = 0;
    version = 0;
}

void WavFile::updatePcmLayout()
{
    if (numberOfChannels_ != 1 && numberOfChannels_ != 2)
    {
        throw WavError(WavErrorCode::unsupportedChannelCount,
                       "unsupported channel count " + std::to_string(numberOfChannels_) +
                           "; only mono and stereo PCM are supported");
    }
    if (sampleRate_ == 0)
    {
        throw WavError(WavErrorCode::invalidSampleRate,
                       "invalid sample rate 0; it must be greater than zero");
    }

    blockAlign = static_cast<std::uint16_t>(numberOfChannels_ * bytesPerSample(bitsPerSample_));
    const auto calculatedByteRate = static_cast<std::uint64_t>(sampleRate_) * blockAlign;
    if (calculatedByteRate > std::numeric_limits<std::uint32_t>::max())
    {
        throw WavError(WavErrorCode::sizeOverflow,
                       "byte rate exceeds the WAV limit for sample rate " +
                           std::to_string(sampleRate_) + " and block alignment " +
                           std::to_string(blockAlign));
    }
    byteRate = static_cast<std::uint32_t>(calculatedByteRate);
}

} // namespace wavlab
