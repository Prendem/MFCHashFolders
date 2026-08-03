#pragma once
#include <span>
#include <vector>
#include <array>
#include <filesystem>
#include <string>
#include <Windows.h>

#include "MD5Hasher.hpp"
#include "SHA2_256Hasher.hpp"
#include "SHA3_256Hasher.hpp"

// for notifying the main thread that a worker thread has completed a hash computation and the future is ready for use.
// wParam - the generation, incremented whenever the user selects a new file set, used to determine if the returned value is stale
// lParam - array index of the relevant directory object
constexpr UINT WM_HASH_COMPLETE = WM_APP + 1;

enum class HashType
{
	MD5,
	SHA2,
	SHA3,
};

struct JobDispatch
{
	int generation;
	int index;
	std::filesystem::path path;
	HashType hashType;
	JobDispatch(int inGeneration, int inIndex, std::filesystem::path inPath, HashType inHashType) : generation{ inGeneration }, index{ inIndex },
		path { inPath }, hashType{ inHashType } {}
};

inline std::wstring toHexString(const std::span<const uint8_t> digest)
{
	std::wostringstream woss;
	woss << std::hex << std::setfill(L'0');
	for (const uint8_t byte : digest)
	{
		woss << std::setw(2) << static_cast<int>(byte);
	}
	return woss.str();
}

template <typename Hasher>
concept HashAlgorithm = requires(Hasher hashObject, std::span<const uint8_t> data, std::vector<uint8_t>& chunk, size_t fileSize)
{
		{ Hasher::digestSize } -> std::convertible_to<size_t>;
		{ hashObject.processChunk(data) };
		requires (requires { hashObject.padFinalChunk(chunk); } || requires { hashObject.padFinalChunk(chunk, fileSize); }); // accepts either function prototype
		{ hashObject.getDigest() } -> std::same_as<std::array<uint8_t, Hasher::digestSize>>;
};

// helper function to ensure the right overload is called
template <typename Hasher>
inline void callPadFinalChunk(Hasher& hasher, std::vector<uint8_t>& chunk, size_t fileSize)
{
	if constexpr (requires { hasher.padFinalChunk(chunk, fileSize); })
	{
		hasher.padFinalChunk(chunk, fileSize);
	}

	else
	{
		hasher.padFinalChunk(chunk);
	}
}


template <HashAlgorithm H>
inline std::array<uint8_t, H::digestSize> computeFileHash(const std::filesystem::path& path)
{
	constexpr size_t chunkGranularity{ 64 * 136 * 128 }; // loweset common multiple of the system allocation granularity (64KB), SHA2/MD5 chunk size(64B) and SHA3 chunk size(136B)
	constexpr size_t maxChunkSize{ 64 * 136 * 4096 }; // 34MB, kind of an arbitrary choice, may increase later

	H hasher{};

	HANDLE fileHandle{ CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL) };

	// TODO expand for all error types that CreateFileW can generate
	if (fileHandle == INVALID_HANDLE_VALUE)
	{
		if (GetLastError() == ERROR_FILE_NOT_FOUND)
			throw std::runtime_error("File " + path.string() + " not found");
		else
			throw std::runtime_error("Another error occured: " + std::to_string(GetLastError()));
	}

	LARGE_INTEGER fileSizeLI;
	GetFileSizeEx(fileHandle, &fileSizeLI);
	size_t fileSize{ static_cast<size_t>(fileSizeLI.QuadPart) };

	if (fileSize == 0)
	{
		std::vector<uint8_t> finalChunk;
		callPadFinalChunk(hasher, finalChunk, fileSize);
		hasher.processChunk(finalChunk);
		CloseHandle(fileHandle);
		return hasher.getDigest();
	}

	HANDLE mapHandle{ CreateFileMappingW(fileHandle, NULL, PAGE_READONLY, 0, 0, NULL) };

	if (mapHandle == NULL)
	{
		CloseHandle(fileHandle);
		throw std::runtime_error("Error creating file mapping\n");
	}

	size_t cursor{ 0 };
	size_t chunkSize{ fileSize < maxChunkSize ? ((fileSize + chunkGranularity - 1) / chunkGranularity) * chunkGranularity : maxChunkSize };

	while (cursor < fileSize)
	{
		size_t bytesRemaining{ fileSize - cursor };

		// not the final chunk, no padding required
		if (bytesRemaining > chunkSize)
		{
			const uint8_t* chunk{ static_cast<const uint8_t*>(MapViewOfFile(mapHandle, FILE_MAP_READ, static_cast<DWORD>(cursor >> 32), static_cast<DWORD>(cursor & 0xFFFFFFFF), chunkSize)) };
			
			// TODO better error handling
			if (!chunk)
			{
				CloseHandle(mapHandle);
				CloseHandle(fileHandle);
				throw std::runtime_error("MapViewOfFile failed");
			}

			hasher.processChunk(std::span<const uint8_t>(chunk, chunkSize));
			cursor += chunkSize;
			UnmapViewOfFile(static_cast<LPCVOID>(chunk));
		}

		// final chunk, needs padding
		else
		{
			const uint8_t* chunk{ static_cast<const uint8_t*>(MapViewOfFile(mapHandle, FILE_MAP_READ, static_cast<DWORD>(cursor >> 32), static_cast<DWORD>(cursor & 0xFFFFFFFF), bytesRemaining)) };

			// TODO better error handling
			if (!chunk)
			{
				CloseHandle(mapHandle);
				CloseHandle(fileHandle);
				throw std::runtime_error("MapViewOfFile failed");
			}

			std::vector<uint8_t> finalChunk(chunk, chunk + bytesRemaining);
			callPadFinalChunk(hasher, finalChunk, fileSize);
			hasher.processChunk(finalChunk);
			cursor += bytesRemaining;
			UnmapViewOfFile(static_cast<LPCVOID>(chunk));
		}
	}

	CloseHandle(mapHandle);
	CloseHandle(fileHandle);
	return hasher.getDigest();
}
