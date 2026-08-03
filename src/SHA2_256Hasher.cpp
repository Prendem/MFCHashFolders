#include "SHA2_256Hasher.hpp"

using namespace Crypto;

std::array<uint32_t, 16> SHA2_256Hasher::getBlockWords(const std::span<const uint8_t>& chunk, int blockNumber)
{
	// for each 512 bit (64 byte) block, break it into 16 32 bit (4 byte) words in big endian form
	std::array<uint32_t, 16> returnArray;
	int blockOffset{ blockNumber * 64 };
	for (int index{ 0 }; index < returnArray.size(); ++index)
	{
		// take each 32 bit word and interpret its value as big endian
		returnArray[index] = static_cast<uint32_t>(chunk[blockOffset + index * 4] << 24) | static_cast<uint32_t>(chunk[blockOffset + index * 4 + 1] << 16) |
			static_cast<uint32_t>(chunk[blockOffset + index * 4 + 2] << 8) | static_cast<uint32_t>(chunk[blockOffset + index * 4 + 3]);
	}

	return returnArray;
}

void SHA2_256Hasher::processBlock(const std::array<uint32_t, 16>& words)
{
	// construct manifest
	std::array<uint32_t, 64> manifest;

	for (int index{ 0 }; index < 16; ++index)
		manifest[index] = words[index];

	for (int index{ 16 }; index < 64; ++index)
		manifest[index] = smallSigma1(manifest[index - 2]) + manifest[index - 7] + smallSigma0(manifest[index - 15]) + manifest[index - 16];

	// initilize working variables
	std::array<uint32_t, 8> workingVariables = m_state;

	// rounds
	for (int index{ 0 }; index < 64; ++index)
	{
		uint32_t t1{ workingVariables[7] + bigSigma1(workingVariables[4]) + ch(workingVariables[4], workingVariables[5], workingVariables[6]) + K[index] + manifest[index] };
		uint32_t t2{ bigSigma0(workingVariables[0]) + maj(workingVariables[0], workingVariables[1], workingVariables[2]) };
		workingVariables[7] = workingVariables[6];
		workingVariables[6] = workingVariables[5];
		workingVariables[5] = workingVariables[4];
		workingVariables[4] = workingVariables[3] + t1;
		workingVariables[3] = workingVariables[2];
		workingVariables[2] = workingVariables[1];
		workingVariables[1] = workingVariables[0];
		workingVariables[0] = t1 + t2;
	}

	// update state
	for (int index{ 0 }; index < m_state.size(); ++index)
		m_state[index] += workingVariables[index];
}

void SHA2_256Hasher::padFinalChunk(std::vector<uint8_t>& chunk, size_t fileSize)
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
		for (int i = 0; i < 8; ++i)
		{
			chunk[blockSize + paddingSize + i] = static_cast<uint8_t>((fileSizeInBits >> (8 * (7 - i))) & 0xFF);
		}
	}
	else
	{
		std::memcpy(&chunk[blockSize + paddingSize], &fileSizeInBits, sizeof(fileSizeInBits));
	}
}

void SHA2_256Hasher::processChunk(std::span<const uint8_t> chunk)
{
	for (int blockNumber{ 0 }; blockNumber * 64 < chunk.size(); ++blockNumber)
	{
		processBlock(getBlockWords(chunk, blockNumber));
	}
}

std::array<uint8_t, SHA2_256Hasher::digestSize> SHA2_256Hasher::getDigest()
{
	std::array<uint8_t, 32> digest;
	for (int index = 0; index < m_state.size(); ++index)
	{
		digest[index * 4] = static_cast<uint8_t>((m_state[index] >> 24) & 0xFF);
		digest[index * 4 + 1] = static_cast<uint8_t>((m_state[index] >> 16) & 0xFF);
		digest[index * 4 + 2] = static_cast<uint8_t>((m_state[index] >> 8) & 0xFF);
		digest[index * 4 + 3] = static_cast<uint8_t>(m_state[index] & 0xFF);
	}

	return digest;
}
