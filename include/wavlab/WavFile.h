#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <span>
#include <stdexcept>
#include <string>
#include <vector>

namespace wavlab
{

enum class WavErrorCode
{
    io,
    invalidFile,
    unsupportedChannelCount,
    invalidSampleRate,
    unsupportedBitsPerSample,
    samplesNotLoaded,
    incompleteSampleFrame,
    sizeOverflow,
};

class WavError : public std::runtime_error
{
  public:
    WavError(WavErrorCode code, std::string message);

    [[nodiscard]] WavErrorCode code() const noexcept;

  private:
    WavErrorCode code_;
};

class WavFile
{
  public:
    [[nodiscard]] static WavFile open(const std::filesystem::path &path);
    [[nodiscard]] static WavFile openMetadata(const std::filesystem::path &path);
    [[nodiscard]] static WavFile create(std::uint16_t numberOfChannels, std::uint32_t sampleRate,
                                        std::uint16_t bitsPerSample);

    void save(const std::filesystem::path &path);

    [[nodiscard]] std::string details() const;
    [[nodiscard]] std::string debugDetails() const;

    [[nodiscard]] std::uint16_t numberOfChannels() const noexcept;
    [[nodiscard]] std::uint32_t sampleRate() const noexcept;
    [[nodiscard]] std::uint16_t bitsPerSample() const noexcept;
    [[nodiscard]] std::size_t sampleCount() const noexcept;
    [[nodiscard]] std::size_t frameCount() const noexcept;
    [[nodiscard]] bool hasLoadedSamples() const noexcept;

    [[nodiscard]] std::span<const double> samples() const;
    [[nodiscard]] std::span<double> samples();

    void setNumberOfChannels(std::uint16_t number);
    void setSamples(std::vector<double> newSamples);

  private:
    WavFile(const std::filesystem::path &path, bool loadSamples);
    WavFile(std::uint16_t numberOfChannels, std::uint32_t sampleRate, std::uint16_t bitsPerSample);

    void validate() const;
    void readBextFields(std::ifstream &file, std::uint32_t chunkSize, bool isFooter);
    void setDefaultBextFields();
    void updatePcmLayout();

    std::size_t lengthInBytes{};

    std::string fileChunkId;
    std::uint32_t fileChunkSize{};
    std::string format;

    std::string formatChunkId;
    std::uint32_t formatChunkSize{};
    std::uint16_t audioFormat{};
    std::uint16_t numberOfChannels_{};
    std::uint32_t sampleRate_{};
    std::uint32_t byteRate{};
    std::uint16_t blockAlign{};
    std::uint16_t bitsPerSample_{};

    bool isBroadcastWav = false;
    bool bextIsFooter = false;

    std::string bextChunkId;
    std::uint32_t bextChunkSize{};
    std::string description;
    std::string originator;
    std::string originatorReference;
    std::string originationDate;
    std::string originationTime;
    std::uint32_t timeReferenceLow{};
    std::uint32_t timeReferenceHigh{};
    std::uint16_t version{};
    std::vector<std::uint8_t> bextExtraData;

    std::string dataChunkId;
    std::uint32_t dataChunkSize{};
    std::streamoff dataChunkStart{};

    std::size_t numberOfSamples{};
    bool samplesLoaded = false;
    std::vector<double> samples_;
};

} // namespace wavlab
