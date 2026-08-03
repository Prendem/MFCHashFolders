#include "SHA3_256Hasher.hpp"
#include <stdexcept>

using namespace Crypto;

void SHA3_256Hasher::processBlock(std::span<const uint8_t> chunk, size_t blockNumber)
{
	size_t blockOffset{ blockNumber * 136 };

	// Bounds check, should never trigger unless my padding is wrong but will leave it in place for now
	if (blockOffset + 136 > chunk.size())
		throw std::runtime_error("Block size exceeds boundary in absorbBlock(). Check padding logic");

	const uint8_t* blockStart{ chunk.data() + blockOffset };
	int relativeOffset{ 0 };
	for (int row{ 0 }; row < 3; ++row)
	{
		for (int column{ 0 }; column < 5; ++column)
		{
			m_state[column][row] ^= *reinterpret_cast<const uint64_t*>(blockStart + relativeOffset * 8);
			++relativeOffset;
		}
	}

	m_state[0][3] ^= *reinterpret_cast<const uint64_t*>(blockStart + relativeOffset * 8);
	++relativeOffset;
	m_state[1][3] ^= *reinterpret_cast<const uint64_t*>(blockStart + relativeOffset * 8);

	//keccak
	for (int round{ 0 }; round < 24; ++round)
	{
		theta();
		rhoPi();
		chi();
		iota(round);
	}
}

void SHA3_256Hasher::padFinalChunk(std::vector<uint8_t>& byteArray)
{
	size_t originalSize{ byteArray.size() };
	size_t paddingSize{ 136 - originalSize % 136 };
	byteArray.resize(originalSize + paddingSize, 0x00);
	byteArray[originalSize] = 0x06;
	byteArray[byteArray.size() - 1] |= 0x80;
}

void SHA3_256Hasher::processChunk(std::span<const uint8_t> chunk)
{
	size_t blockCount{ chunk.size() / 136 };
	for (size_t blockNumber{ 0 }; blockNumber < blockCount; ++blockNumber)
	{
		processBlock(chunk, blockNumber);
	}
}

std::array<uint8_t, SHA3_256Hasher::digestSize> SHA3_256Hasher::getDigest()
{
	std::array<uint8_t, 32> digest;
	for (int column{ 0 }; column < 4; ++column)
		std::memcpy(&digest[8 * column], &m_state[column][0], 8);

	return digest;
}
