#include "MD5Hasher.hpp"
#include <bit>

using namespace Crypto;

std::array<uint32_t, 16> MD5Hasher::getBlockWords(std::span<const uint8_t> chunk, int blockNumber)
{
    std::array<uint32_t, 16> words;
    int blockOffset{ blockNumber * 64 };
    for (int index{ 0 }; index < words.size(); ++index)
    {
        std::memcpy(&words[index], &chunk[blockOffset + index * 4], sizeof(uint32_t));
    }

    return words;
}

void MD5Hasher::processBlock(const std::array<uint32_t, 16>& words)
{
    // TODO decide on naming convention/style for working variables between this and the SHA3 approach
    // working variables
    uint32_t a{ m_state[0] }, b{ m_state[1] }, c{ m_state[2] }, d{ m_state[3] };

    for (int round{ 0 }; round < 64; ++round)
    {
        uint32_t f, g;

        if (round < 16)
        {
            f = F(b, c, d);
            g = round;
        }

        else if (round < 32)
        {
            f = G(b, c, d);
            g = (5 * round + 1) % 16;
        }

        else if (round < 48)
        {
            f = H(b, c, d);
            g = (3 * round + 5) % 16;
        }

        else
        {
            f = I(b, c, d);
            g = (7 * round) % 16;
        }

        uint32_t temp{ d };
        d = c;
        c = b;
        b = b + std::rotl(a + f + K[round] + words[g], s[round]);
        a = temp;
    }

    m_state[0] += a;
    m_state[1] += b;
    m_state[2] += c;
    m_state[3] += d;
}

void MD5Hasher::padFinalChunk(std::vector<uint8_t>& chunk, size_t fileSize)
{
    size_t originalSize{ chunk.size() };
    size_t blockSize{ originalSize + 1 }; // +1 for the 0x80 byte
    size_t sizeMod64{ blockSize % 64 };
    size_t paddingSize{ (sizeMod64 > 56) ? (56 + 64 - sizeMod64) : (56 - sizeMod64) };
    chunk.resize(blockSize + paddingSize + 8, 0x00); // +8 for the 64 bit length
    chunk[originalSize] = 0x80;

    const uint64_t fileSizeInBits{ fileSize * 8 };

    if (std::endian::native == std::endian::little)
    {
        std::memcpy(&chunk[blockSize + paddingSize], &fileSizeInBits, sizeof(fileSizeInBits));
    }
    else
    {
        for (int i = 0; i < 8; ++i)
        {
            chunk[blockSize + paddingSize + i] = static_cast<uint8_t>((fileSizeInBits >> (8 * i)) & 0xFF);
        }
    }
}

void MD5Hasher::processChunk(std::span<const uint8_t> chunk)
{
    for (int blockNumber{ 0 }; blockNumber * 64 < chunk.size(); ++blockNumber)
    {
        processBlock(getBlockWords(chunk, blockNumber));
    }
}

std::array<uint8_t, MD5Hasher::digestSize> MD5Hasher::getDigest()
{
    std::array<uint8_t, digestSize> digest;

    for (int i = 0; i < 4; ++i)
    {
        uint32_t word = m_state[i];
        digest[i * 4 + 0] = static_cast<uint8_t>(word & 0xFF);
        digest[i * 4 + 1] = static_cast<uint8_t>((word >> 8) & 0xFF);
        digest[i * 4 + 2] = static_cast<uint8_t>((word >> 16) & 0xFF);
        digest[i * 4 + 3] = static_cast<uint8_t>((word >> 24) & 0xFF);
    }

    return digest;
}
