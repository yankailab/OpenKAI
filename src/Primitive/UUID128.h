#ifndef OpenKAI_src_Primitive_UUID128_H_
#define OpenKAI_src_Primitive_UUID128_H_

#include <stdint.h>

namespace kai
{
	union UUID128
	{
		uint8_t m_uint8[16];
		uint16_t m_uint16[8];
		uint32_t m_uint32[4];
		uint64_t m_uint64[2];

		UUID128() = default;

		constexpr bool operator==(uint64_t rhs) const noexcept
		{
			return m_uint64[0] == rhs && m_uint64[1] == 0;
		}

		constexpr UUID128(uint64_t low, uint64_t high = 0) noexcept
			: m_uint64{low, high}
		{
		}

		UUID128 &operator=(int rhs) noexcept
		{
			m_uint64[0] = static_cast<uint64_t>(rhs);
			m_uint64[1] = 0;
			return *this;
		}

		UUID128 &operator=(uint8_t rhs) noexcept
		{
			m_uint64[0] = rhs;
			m_uint64[1] = 0;
			return *this;
		}

		UUID128 &operator=(uint16_t rhs) noexcept
		{
			m_uint64[0] = rhs;
			m_uint64[1] = 0;
			return *this;
		}

		UUID128 &operator=(uint32_t rhs) noexcept
		{
			m_uint64[0] = rhs;
			m_uint64[1] = 0;
			return *this;
		}

		UUID128 &operator=(uint64_t rhs) noexcept
		{
			m_uint64[0] = rhs;
			m_uint64[1] = 0;
			return *this;
		}

		UUID128 &operator|=(const UUID128 &rhs) noexcept
		{
			m_uint64[0] |= rhs.m_uint64[0];
			m_uint64[1] |= rhs.m_uint64[1];
			return *this;
		}

		UUID128 &operator|=(uint64_t rhs) noexcept
		{
			m_uint64[0] |= rhs;
			return *this;
		}

		UUID128 operator<<(unsigned int shift) const noexcept
		{
			if (shift == 0)
				return *this;
			if (shift >= 128)
				return {};

			// Word 0 holds the low 64 bits; word 1 holds the high 64 bits.
			UUID128 r;

			if (shift < 64)
			{
				r.m_uint64[0] = m_uint64[0] << shift;
				r.m_uint64[1] = (m_uint64[1] << shift) | (m_uint64[0] >> (64 - shift));
			}
			else
			{
				r.m_uint64[0] = 0;
				r.m_uint64[1] = m_uint64[0] << (shift - 64);
			}

			return r;
		}
	};

}
#endif
