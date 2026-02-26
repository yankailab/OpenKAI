#ifndef OpenKAI_src_Primitive_vBit_H_
#define OpenKAI_src_Primitive_vBit_H_

#include "../Base/platform.h"

namespace kai
{

    class vBit
    {
    public:
        vBit()
        {
            m_nBits = 0;
            m_nWords = 0;
        }

        vBit(size_t nBits) { init(nBits); }

        // init(nBits): allocate and clear
        void init(size_t nBits)
        {
            m_nBits = nBits;
            m_nWords = (nBits + 63) >> 6;
            m_vData.assign(m_nWords, 0ULL);
        }

        size_t size_bits() const { return m_nBits; }
        size_t size_words() const { return m_nWords; }

        uint64_t *raw() { return m_vData.data(); }
        const uint64_t *raw() const { return m_vData.data(); }

        void reset() { std::fill(m_vData.begin(), m_vData.end(), 0ULL); }

        inline void set(size_t bit)
        {
            if (bit >= m_nBits)
                return;
            m_vData[bit >> 6] |= (1ULL << (bit & 63));
        }

        inline void clear(size_t bit)
        {
            if (bit >= m_nBits)
                return;
            m_vData[bit >> 6] &= ~(1ULL << (bit & 63));
        }

        inline bool bit(size_t bit) const
        {
            if (bit >= m_nBits)
                return false;
            return (m_vData[bit >> 6] >> (bit & 63)) & 1ULL;
        }

        // Bitwise ops (in-place)
        // NOTE: if sizes differ, operates on min word count.
        vBit &operator^=(const vBit &v)
        {
            const size_t m = minWords(v);
            for (size_t i = 0; i < m; ++i)
                m_vData[i] ^= v.m_vData[i];
            maskLastWord();
            return *this;
        }

        vBit &operator&=(const vBit &v)
        {
            const size_t m = minWords(v);
            for (size_t i = 0; i < m; ++i)
                m_vData[i] &= v.m_vData[i];
            maskLastWord();
            return *this;
        }

        vBit &operator|=(const vBit &v)
        {
            const size_t m = minWords(v);
            for (size_t i = 0; i < m; ++i)
                m_vData[i] |= v.m_vData[i];
            maskLastWord();
            return *this;
        }

        friend vBit operator^(vBit a, const vBit &b)
        {
            a ^= b;
            return a;
        }
        friend vBit operator&(vBit a, const vBit &b)
        {
            a &= b;
            return a;
        }
        friend vBit operator|(vBit a, const vBit &b)
        {
            a |= b;
            return a;
        }

        // Equality / numeric comparisons (big unsigned)
        bool operator==(const vBit &v) const
        {
            if (m_nBits != v.m_nBits)
                return false;
            for (size_t i = 0; i < m_nWords; ++i)
            {
                if (maskedWord(i) != v.maskedWord(i))
                    return false;
            }
            return true;
        }

        bool operator!=(const vBit &v) const { return !(*this == v); }

        bool operator<(const vBit &v) const
        {
            if (m_nBits != v.m_nBits)
                return m_nBits < v.m_nBits;
            for (size_t i = m_nWords; i-- > 0;)
            {
                const uint64_t a = maskedWord(i);
                const uint64_t b = v.maskedWord(i);
                if (a < b)
                    return true;
                if (a > b)
                    return false;
            }
            return false;
        }

        bool operator>(const vBit &v) const { return v < *this; }
        bool operator<=(const vBit &v) const { return !(*this > v); }
        bool operator>=(const vBit &v) const { return !(*this < v); }

        // Popcount / predicates
        size_t popcount() const
        {
            if (m_nWords == 0)
                return 0;
            size_t c = 0;
            for (size_t i = 0; i + 1 < m_nWords; ++i)
                c += popcount64_(m_vData[i]);
            c += popcount64_(maskedWord(m_nWords - 1));
            return c;
        }

        bool any() const
        {
            if (m_nWords == 0)
                return false;
            for (size_t i = 0; i + 1 < m_nWords; ++i)
                if (m_vData[i])
                    return true;
            return maskedWord(m_nWords - 1) != 0;
        }

        bool none() const { return !any(); }

        bool all() const
        {
            if (m_nWords == 0)
                return true;
            for (size_t i = 0; i + 1 < m_nWords; ++i)
                if (m_vData[i] != ~0ULL)
                    return false;

            const size_t used = m_nBits & 63;
            if (used == 0)
                return m_vData[m_nWords - 1] == ~0ULL;
            const uint64_t mask = (1ULL << used) - 1ULL;
            return (m_vData[m_nWords - 1] & mask) == mask;
        }

        // Mask relations
        bool subsetOf(const vBit &v) const
        {
            if (m_nBits != v.m_nBits)
                return false;
            for (size_t i = 0; i + 1 < m_nWords; ++i)
            {
                if (m_vData[i] & ~v.m_vData[i])
                    return false;
            }
            const uint64_t a = maskedWord(m_nWords - 1);
            const uint64_t b = v.maskedWord(m_nWords - 1);
            return (a & ~b) == 0;
        }

        bool intersects(const vBit &v) const
        {
            if (m_nBits != v.m_nBits)
                return false;
            for (size_t i = 0; i + 1 < m_nWords; ++i)
            {
                if (m_vData[i] & v.m_vData[i])
                    return true;
            }
            return (maskedWord(m_nWords - 1) & v.maskedWord(m_nWords - 1)) != 0;
        }

        // Shifts
        void shiftL(size_t k)
        {
            if (k == 0 || m_nWords == 0)
                return;
            if (k >= m_nBits)
            {
                reset();
                return;
            }

            const size_t wshift = k >> 6;
            const unsigned bshift = (unsigned)(k & 63);

            if (wshift)
            {
                for (size_t i = m_nWords; i-- > 0;)
                {
                    m_vData[i] = (i >= wshift) ? m_vData[i - wshift] : 0ULL;
                }
            }
            if (bshift)
            {
                for (size_t i = m_nWords; i-- > 0;)
                {
                    const uint64_t hi = m_vData[i] << bshift;
                    const uint64_t lo = (i > 0) ? (m_vData[i - 1] >> (64 - bshift)) : 0ULL;
                    m_vData[i] = hi | lo;
                }
            }
            maskLastWord();
        }

        void shiftR(size_t k)
        {
            if (k == 0 || m_nWords == 0)
                return;
            if (k >= m_nBits)
            {
                reset();
                return;
            }

            const size_t wshift = k >> 6;
            const unsigned bshift = (unsigned)(k & 63);

            if (wshift)
            {
                for (size_t i = 0; i < m_nWords; ++i)
                {
                    m_vData[i] = (i + wshift < m_nWords) ? m_vData[i + wshift] : 0ULL;
                }
            }
            if (bshift)
            {
                for (size_t i = 0; i < m_nWords; ++i)
                {
                    const uint64_t lo = m_vData[i] >> bshift;
                    const uint64_t hi = (i + 1 < m_nWords) ? (m_vData[i + 1] << (64 - bshift)) : 0ULL;
                    m_vData[i] = lo | hi;
                }
            }
            maskLastWord();
        }

        vBit &operator<<=(size_t k)
        {
            shiftL(k);
            return *this;
        }
        vBit &operator>>=(size_t k)
        {
            shiftR(k);
            return *this;
        }
        friend vBit operator<<(vBit a, size_t k)
        {
            a.shiftL(k);
            return a;
        }
        friend vBit operator>>(vBit a, size_t k)
        {
            a.shiftR(k);
            return a;
        }

        // Bit scanning (returns m_nBits if not found)
        size_t findFirstOne() const
        {
            if (m_nWords == 0)
                return m_nBits;
            for (size_t wi = 0; wi + 1 < m_nWords; ++wi)
            {
                uint64_t w = m_vData[wi];
                if (w)
                    return (wi << 6) + ctz64_(w);
            }
            uint64_t w = maskedWord(m_nWords - 1);
            if (!w)
                return m_nBits;
            return ((m_nWords - 1) << 6) + ctz64_(w);
        }

        size_t findNextOne(size_t pos) const
        {
            if (m_nWords == 0)
                return m_nBits;
            if (pos + 1 >= m_nBits)
                return m_nBits;

            size_t bit = pos + 1;
            size_t wi = bit >> 6;
            unsigned off = (unsigned)(bit & 63);

            if (wi < m_nWords)
            {
                uint64_t w = (wi + 1 == m_nWords) ? maskedWord(wi) : m_vData[wi];
                w &= (~0ULL << off);
                if (w)
                    return (wi << 6) + ctz64_(w);
                ++wi;
            }

            for (; wi + 1 < m_nWords; ++wi)
            {
                uint64_t w = m_vData[wi];
                if (w)
                    return (wi << 6) + ctz64_(w);
            }

            if (wi < m_nWords)
            {
                uint64_t w = maskedWord(wi);
                if (w)
                    return (wi << 6) + ctz64_(w);
            }
            return m_nBits;
        }

        size_t findLastOne() const
        {
            if (m_nWords == 0)
                return m_nBits;
            for (size_t wi = m_nWords; wi-- > 0;)
            {
                uint64_t w = (wi + 1 == m_nWords) ? maskedWord(wi) : m_vData[wi];
                if (!w)
                    continue;
                return (wi << 6) + (63u - clz64_(w));
            }
            return m_nBits;
        }

        // Hash (64-bit)
        uint64_t hash64() const
        {
            uint64_t h = 0x9e3779b97f4a7c15ULL ^ (uint64_t)m_nBits;
            for (size_t i = 0; i + 1 < m_nWords; ++i)
                h = mix_(h ^ m_vData[i]);
            if (m_nWords)
                h = mix_(h ^ maskedWord(m_nWords - 1));
            return h;
        }

        // Batch XOR reduce
        static void xorReduce(vBit &out, const vBit *arr, size_t count)
        {
            out.reset();
            if (count == 0 || arr == nullptr)
                return;

            const size_t m = out.m_nWords;
            uint64_t *dst = out.m_vData.data();

            for (size_t i = 0; i < count; ++i)
            {
                const size_t mm = (m < arr[i].m_nWords) ? m : arr[i].m_nWords;
                for (size_t j = 0; j < mm; ++j)
                    dst[j] ^= arr[i].m_vData[j];
            }
            out.maskLastWord();
        }

        static void xorReduceInplace(vBit &out, const vBit *arr, size_t count)
        {
            if (count == 0 || arr == nullptr)
                return;

            const size_t m = out.m_nWords;
            uint64_t *dst = out.m_vData.data();

            for (size_t i = 0; i < count; ++i)
            {
                const size_t mm = (m < arr[i].m_nWords) ? m : arr[i].m_nWords;
                for (size_t j = 0; j < mm; ++j)
                    dst[j] ^= arr[i].m_vData[j];
            }
            out.maskLastWord();
        }

    private:
        size_t minWords(const vBit &v) const
        {
            return (m_nWords < v.m_nWords) ? m_nWords : v.m_nWords;
        }

        void maskLastWord()
        {
            if (m_nWords == 0)
                return;
            const size_t used = m_nBits & 63;
            if (used == 0)
                return;
            const uint64_t mask = (1ULL << used) - 1ULL;
            m_vData[m_nWords - 1] &= mask;
        }

        uint64_t maskedWord(size_t i) const
        {
            if (i + 1 != m_nWords)
                return m_vData[i];
            const size_t used = m_nBits & 63;
            if (used == 0)
                return m_vData[i];
            const uint64_t mask = (1ULL << used) - 1ULL;
            return m_vData[i] & mask;
        }

        static inline uint64_t mix_(uint64_t x)
        {
            x ^= x >> 30;
            x *= 0xbf58476d1ce4e5b9ULL;
            x ^= x >> 27;
            x *= 0x94d049bb133111ebULL;
            x ^= x >> 31;
            return x;
        }

        static inline uint32_t popcount64_(uint64_t x)
        {
            return (uint32_t)__builtin_popcountll(x);
        }

        static inline unsigned ctz64_(uint64_t x)
        {
            return (unsigned)__builtin_ctzll(x); // x must be nonzero
        }

        static inline unsigned clz64_(uint64_t x)
        {
            return (unsigned)__builtin_clzll(x); // x must be nonzero
        }

    private:
        size_t m_nBits;
        size_t m_nWords;
        std::vector<uint64_t> m_vData;
    };
}

#endif