#include "BinaryHelpers.h"
#include "ConversionHelpers.h"
#include <chrono>
#include <fstream>
#include <limits>
#include <sstream>
#include <string>
#include <utility>
#include <vector>
#include <wavlab/WavFile.h>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

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

std::string readFixedString(std::istream &stream, std::size_t size)
{
    std::string value = BinaryHelpers::readString(stream, size);
    const auto terminator = value.find('\0');
    if (terminator != std::string::npos)
    {
        value.resize(terminator);
    }
    return value;
}

std::uint64_t serializedChunkSize(std::uint32_t dataSize)
{
    return 8ULL + dataSize + (dataSize % 2U);
}

void writePadding(std::ostream &stream, std::uint32_t dataSize)
{
    if (dataSize % 2U != 0)
    {
        BinaryHelpers::writeUInt8(stream, 0);
    }
}

std::filesystem::path temporarySibling(const std::filesystem::path &destination)
{
    const auto suffix = std::chrono::steady_clock::now().time_since_epoch().count();
    auto temporary = destination;
    temporary += ".wavlab-tmp-" + std::to_string(suffix);
    return temporary;
}

class TemporaryFileGuard
{
  public:
    explicit TemporaryFileGuard(std::filesystem::path path) : path_(std::move(path)) {}
    ~TemporaryFileGuard()
    {
        if (active_)
        {
            std::error_code ignored;
            std::filesystem::remove(path_, ignored);
        }
    }
    TemporaryFileGuard(const TemporaryFileGuard &) = delete;
    TemporaryFileGuard &operator=(const TemporaryFileGuard &) = delete;
    void release() noexcept { active_ = false; }

  private:
    std::filesystem::path path_;
    bool active_ = true;
};

void replaceFile(const std::filesystem::path &temporary, const std::filesystem::path &destination)
{
#ifdef _WIN32
    if (!MoveFileExW(temporary.c_str(), destination.c_str(),
                     MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
    {
        throw WavError(WavErrorCode::io, "could not replace WAV file: " + destination.string() +
                                             " (Windows error " + std::to_string(GetLastError()) +
                                             ")");
    }
#else
    std::error_code error;
    std::filesystem::rename(temporary, destination, error);
    if (error)
    {
        throw WavError(WavErrorCode::io, "could not replace WAV file: " + destination.string() +
                                             ": " + error.message());
    }
#endif
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
    const auto fileLength = infile.tellg();
    if (fileLength < 12)
    {
        throw WavError(WavErrorCode::invalidFile,
                       "invalid WAV file: file is too short to contain a RIFF header");
    }
    if (static_cast<std::uint64_t>(fileLength) > std::numeric_limits<std::size_t>::max())
    {
        throw WavError(WavErrorCode::sizeOverflow,
                       "WAV file length cannot be represented on this platform");
    }
    lengthInBytes = static_cast<std::size_t>(fileLength);
    infile.seekg(0, std::ios::beg);

    fileChunkId = BinaryHelpers::readString(infile, 4);
    fileChunkSize = BinaryHelpers::readUInt32(infile);
    format = BinaryHelpers::readString(infile, 4);

    if (fileChunkId != "RIFF")
    {
        throw WavError(WavErrorCode::invalidFile,
                       "invalid WAV file: expected RIFF identifier, found " + fileChunkId);
    }
    if (format != "WAVE")
    {
        throw WavError(WavErrorCode::invalidFile,
                       "invalid WAV file: expected WAVE format, found " + format);
    }
    if (static_cast<std::uint64_t>(fileChunkSize) + 8ULL != lengthInBytes)
    {
        throw WavError(WavErrorCode::invalidFile, "invalid WAV file: RIFF header declares " +
                                                      std::to_string(fileChunkSize + 8ULL) +
                                                      " bytes, but the file contains " +
                                                      std::to_string(lengthInBytes));
    }

    bool foundFormat = false;
    bool foundData = false;
    while (static_cast<std::size_t>(infile.tellg()) < lengthInBytes)
    {
        const auto headerStart = static_cast<std::size_t>(infile.tellg());
        if (lengthInBytes - headerStart < 8)
        {
            throw WavError(WavErrorCode::invalidFile, "invalid WAV file: truncated chunk header");
        }

        const std::string chunkId = BinaryHelpers::readString(infile, 4);
        const std::uint32_t chunkSize = BinaryHelpers::readUInt32(infile);
        const auto chunkDataStart = static_cast<std::uint64_t>(infile.tellg());
        const auto chunkDataEnd = chunkDataStart + chunkSize;
        const auto paddedEnd = chunkDataEnd + (chunkSize % 2U);
        if (paddedEnd > lengthInBytes)
        {
            throw WavError(WavErrorCode::invalidFile,
                           "invalid WAV file: " + chunkId +
                               " chunk extends beyond the end of the file");
        }

        if (chunkId == "fmt ")
        {
            if (foundFormat)
            {
                throw WavError(WavErrorCode::invalidFile, "invalid WAV file: multiple fmt chunks");
            }
            if (chunkSize < 16)
            {
                throw WavError(WavErrorCode::invalidFile,
                               "invalid WAV file: fmt chunk is smaller than 16 bytes");
            }
            foundFormat = true;
            formatChunkId = chunkId;
            formatChunkSize = chunkSize;
            audioFormat = BinaryHelpers::readUInt16(infile);
            numberOfChannels_ = BinaryHelpers::readUInt16(infile);
            sampleRate_ = BinaryHelpers::readUInt32(infile);
            byteRate = BinaryHelpers::readUInt32(infile);
            blockAlign = BinaryHelpers::readUInt16(infile);
            bitsPerSample_ = BinaryHelpers::readUInt16(infile);
        }
        else if (chunkId == "data")
        {
            if (foundData)
            {
                throw WavError(WavErrorCode::invalidFile, "invalid WAV file: multiple data chunks");
            }
            foundData = true;
            dataChunkId = chunkId;
            dataChunkSize = chunkSize;
            dataChunkStart = static_cast<std::streamoff>(chunkDataStart);
        }
        else if (chunkId == "bext")
        {
            if (isBroadcastWav)
            {
                throw WavError(WavErrorCode::invalidFile, "invalid WAV file: multiple BEXT chunks");
            }
            isBroadcastWav = true;
            readBextFields(infile, chunkSize, foundData);
        }

        infile.seekg(static_cast<std::streamoff>(paddedEnd));
        if (!infile)
        {
            throw WavError(WavErrorCode::io, "failed while seeking through WAV chunks");
        }
    }

    if (!foundFormat)
    {
        throw WavError(WavErrorCode::invalidFile, "invalid WAV file: missing fmt chunk");
    }
    if (!foundData)
    {
        throw WavError(WavErrorCode::invalidFile, "invalid WAV file: missing data chunk");
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
                samples_[i] = ConversionHelpers::eightBitToDouble(BinaryHelpers::readUInt8(infile));
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

    const auto sampleWidth = bytesPerSample(bitsPerSample_);
    if (samples_.size() > std::numeric_limits<std::uint32_t>::max() / sampleWidth)
    {
        throw WavError(WavErrorCode::sizeOverflow, "sample data is too large for a WAV file");
    }
    const auto dataSize = samples_.size() * sampleWidth;

    formatChunkSize = 16;
    dataChunkSize = static_cast<std::uint32_t>(dataSize);
    if (isBroadcastWav && bextExtraData.size() > std::numeric_limits<std::uint32_t>::max() - 348ULL)
    {
        throw WavError(WavErrorCode::sizeOverflow, "BEXT chunk is too large for a WAV file");
    }
    const auto calculatedBextSize = 348ULL + bextExtraData.size();
    bextChunkSize = isBroadcastWav ? static_cast<std::uint32_t>(calculatedBextSize) : 0;
    std::uint64_t totalFileSize =
        12ULL + serializedChunkSize(16) + serializedChunkSize(dataChunkSize);
    if (isBroadcastWav)
    {
        totalFileSize += serializedChunkSize(bextChunkSize);
    }
    if (totalFileSize - 8ULL > std::numeric_limits<std::uint32_t>::max())
    {
        throw WavError(WavErrorCode::sizeOverflow, "RIFF data is too large for a WAV file");
    }
    fileChunkSize = static_cast<std::uint32_t>(totalFileSize - 8ULL);
    lengthInBytes = static_cast<std::size_t>(totalFileSize);

    validate();

    const auto temporaryPath = temporarySibling(path);
    TemporaryFileGuard temporaryGuard(temporaryPath);
    std::ofstream outfile(temporaryPath, std::ofstream::binary | std::ofstream::trunc);

    if (!outfile)
    {
        throw WavError(WavErrorCode::io, "could not open WAV file for writing: " + path.string());
    }

    BinaryHelpers::writeString(outfile, fileChunkId);
    BinaryHelpers::writeUInt32(outfile, fileChunkSize);
    BinaryHelpers::writeString(outfile, format);

    BinaryHelpers::writeString(outfile, formatChunkId);
    BinaryHelpers::writeUInt32(outfile, formatChunkSize);
    BinaryHelpers::writeUInt16(outfile, audioFormat);
    BinaryHelpers::writeUInt16(outfile, numberOfChannels_);
    BinaryHelpers::writeUInt32(outfile, sampleRate_);
    BinaryHelpers::writeUInt32(outfile, byteRate);
    BinaryHelpers::writeUInt16(outfile, blockAlign);
    BinaryHelpers::writeUInt16(outfile, bitsPerSample_);

    const auto writeBext = [&outfile, this]
    {
        BinaryHelpers::writeString(outfile, bextChunkId);
        BinaryHelpers::writeUInt32(outfile, bextChunkSize);

        BinaryHelpers::writeFixedString(outfile, description, 256);
        BinaryHelpers::writeFixedString(outfile, originator, 32);
        BinaryHelpers::writeFixedString(outfile, originatorReference, 32);
        BinaryHelpers::writeFixedString(outfile, originationDate, 10);
        BinaryHelpers::writeFixedString(outfile, originationTime, 8);

        BinaryHelpers::writeUInt32(outfile, timeReferenceLow);
        BinaryHelpers::writeUInt32(outfile, timeReferenceHigh);
        BinaryHelpers::writeUInt16(outfile, version);
        BinaryHelpers::writeBytes(outfile, bextExtraData);
        writePadding(outfile, bextChunkSize);
    };

    if (isBroadcastWav && !bextIsFooter)
    {
        writeBext();
    }

    BinaryHelpers::writeString(outfile, dataChunkId);
    BinaryHelpers::writeUInt32(outfile, dataChunkSize);

    if (bitsPerSample_ == 8)
    {
        for (size_t i = 0; i < numberOfSamples; i++)
        {
            BinaryHelpers::writeUInt8(outfile, ConversionHelpers::doubleToEightBit(samples_[i]));
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

    writePadding(outfile, dataChunkSize);
    if (isBroadcastWav && bextIsFooter)
    {
        writeBext();
    }

    outfile.close();
    if (!outfile)
    {
        throw WavError(WavErrorCode::io, "failed while writing WAV file: " + path.string());
    }
    replaceFile(temporaryPath, path);
    temporaryGuard.release();
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
void WavFile::readBextFields(std::ifstream &file, std::uint32_t chunkSize, bool isFooter)
{
    constexpr std::uint32_t knownFieldsSize = 348;
    if (chunkSize < knownFieldsSize)
    {
        throw WavError(WavErrorCode::invalidFile,
                       "invalid WAV file: BEXT chunk is smaller than 348 bytes");
    }
    bextChunkId = "bext";
    bextChunkSize = chunkSize;
    bextIsFooter = isFooter;

    description = readFixedString(file, 256);
    originator = readFixedString(file, 32);
    originatorReference = readFixedString(file, 32);
    originationDate = readFixedString(file, 10);
    originationTime = readFixedString(file, 8);
    timeReferenceLow = BinaryHelpers::readUInt32(file);
    timeReferenceHigh = BinaryHelpers::readUInt32(file);
    version = BinaryHelpers::readUInt16(file);
    bextExtraData = BinaryHelpers::readBytes(file, chunkSize - knownFieldsSize);
}

void WavFile::setDefaultBextFields()
{
    isBroadcastWav = false;
    bextIsFooter = false;
    bextChunkId = "bext";
    description = "";
    originator = "";
    originatorReference = "";
    originationDate = "";
    originationTime = "";
    timeReferenceLow = 0;
    timeReferenceHigh = 0;
    version = 0;
    bextExtraData.clear();
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
