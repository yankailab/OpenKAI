#ifndef OpenKAI_src_Universe_Geometry_PointCloud_Pipeline_PCstreamProtocol_H_
#define OpenKAI_src_Universe_Geometry_PointCloud_Pipeline_PCstreamProtocol_H_

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>

namespace kai
{
	namespace pcstream
	{
		// PCL2 only. All values are little-endian, with no native struct padding.
		// Header: magic, packet bytes (u32), stamp (u64),
		// total points (u32), first point (u32). Records: XYZRGB (f32), stamp (u64).
		constexpr uint8_t magic[4] = {'P', 'C', 'L', '2'};
		constexpr size_t headerBytes = 24;
		constexpr size_t pointBytes = 32;
		constexpr size_t maxPacketBytes = 2000;

		inline void packUint(uint8_t *pBytes, uint64_t value, size_t count)
		{
			for (size_t i = 0; i < count; ++i)
			{
				pBytes[i] = static_cast<uint8_t>(value >> (i * 8));
			}
		}

		inline uint64_t unpackUint(const uint8_t *pBytes, size_t count)
		{
			uint64_t value = 0;
			for (size_t i = 0; i < count; ++i)
			{
				value |= static_cast<uint64_t>(pBytes[i]) << (i * 8);
			}
			return value;
		}

		inline void packFloat(uint8_t *pBytes, float value)
		{
			static_assert(sizeof(float) == 4 && std::numeric_limits<float>::is_iec559,
				"PCL2 requires IEEE 754 float32");
			uint32_t bits;
			std::memcpy(&bits, &value, sizeof(bits));
			packUint(pBytes, bits, sizeof(bits));
		}

		inline float unpackFloat(const uint8_t *pBytes)
		{
			const uint32_t bits = static_cast<uint32_t>(unpackUint(pBytes, 4));
			float value;
			std::memcpy(&value, &bits, sizeof(value));
			return value;
		}
	}
}
#endif
