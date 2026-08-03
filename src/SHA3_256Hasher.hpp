#pragma once
#include <array>
#include <span>
#include <bit>
#include <vector>

namespace Crypto
{
	class SHA3_256Hasher
	{
	private:

		static constexpr std::array<std::array<int, 5>, 5> s_rotationAmounts
		{
			{
				{0, 36, 3, 41, 18},
				{1, 44, 10, 45, 2},
				{62, 6, 43, 15, 61},
				{28, 55, 25, 21, 56},
				{27, 20, 39, 8, 14}
			}
		};

		static constexpr std::array<uint64_t, 24> s_roundConstants
		{
			0x0000000000000001, 0x0000000000008082, 0x800000000000808A, 0x8000000080008000,
			0x000000000000808B, 0x0000000080000001, 0x8000000080008081, 0x8000000000008009,
			0x000000000000008A, 0x0000000000000088, 0x0000000080008009, 0x000000008000000A,
			0x000000008000808B, 0x800000000000008B, 0x8000000000008089, 0x8000000000008003,
			0x8000000000008002, 0x8000000000000080, 0x000000000000800A, 0x800000008000000A,
			0x8000000080008081, 0x8000000000008080, 0x0000000080000001, 0x8000000080008008
		};

		std::array<std::array<uint64_t, 5>, 5> m_state{};

		// keccak functions, implicitly inline 

		void theta()
		{
			std::array<uint64_t, 5> parity{};
			std::array<uint64_t, 5> corrections{};

			// step 1 column parities
			for (int column{ 0 }; column < 5; ++column)
				parity[column] = m_state[column][0] ^ m_state[column][1] ^ m_state[column][2] ^ m_state[column][3] ^ m_state[column][4];

			// step 2 correction terms
			for (int column{ 0 }; column < 5; ++column)
				corrections[column] = parity[(column + 4) % 5] ^ std::rotl(parity[(column + 1) % 5], 1);

			// step 3 apply corrections
			for (int column{ 0 }; column < 5; ++column)
				for (int row{ 0 }; row < 5; ++row)
					m_state[column][row] ^= corrections[column];
		}

		// it is more efficient to combine the rho and pi steps rather than do them seperately
		void rhoPi()
		{
			std::array<std::array<uint64_t, 5>, 5> tempState{};

			for (int column{ 0 }; column < 5; ++column)
				for (int row{ 0 }; row < 5; ++row)
				{
					int targetColumn{ row };
					int targetRow{ (2 * column + 3 * row) % 5 };
					tempState[targetColumn][targetRow] = std::rotl(m_state[column][row], s_rotationAmounts[column][row]);
				}

			m_state = tempState;
		}

		void chi()
		{
			std::array<uint64_t, 5> tempRow{};

			for (int row{ 0 }; row < 5; ++row)
			{
				for (int column{ 0 }; column < 5; ++column)
					tempRow[column] = m_state[column][row];

				for (int column{ 0 }; column < 5; ++column)
					m_state[column][row] = tempRow[column] ^ (~tempRow[(column + 1) % 5] & tempRow[(column + 2) % 5]);
			}
		}

		void iota(int round)
		{
			m_state[0][0] ^= s_roundConstants[round];
		}

		void processBlock(std::span<const uint8_t> block, size_t blockNumber);

	public:

		static constexpr size_t digestSize{ 32 };

		void padFinalChunk(std::vector<uint8_t>& byteArray);
		void processChunk(std::span<const uint8_t> chunk);
		std::array<uint8_t, digestSize> getDigest();
	};
}