#ifndef OpenKAI_src_Primitive_vLbit_H_
#define OpenKAI_src_Primitive_vLbit_H_

#include "../Base/platform.h"
#include <limits>

using namespace std;

namespace kai
{
    class vLbit
    {
    public:
        vLbit(void)
        {
            clear();
        }

        void clear(void)
        {
            m_vP.clear();
        }

        const vector<uint64_t>& getVp(void) const
        {
            return m_vP;
        }

        // set bit = 1
        inline void set(uint64_t bit)
        {
            const uint64_t b = bit;
            auto it = lower_bound(m_vP.begin(), m_vP.end(), b);
            if (it == m_vP.end() || *it != b)
                m_vP.insert(it, b);
        }

        // clear bit = 0
        inline void unset(uint64_t bit)
        {
            const uint64_t b = bit;
            auto it = lower_bound(m_vP.begin(), m_vP.end(), b);
            if (it != m_vP.end() && *it == b)
                m_vP.erase(it);
        }

        // return 1 if set, else 0
        inline uint8_t get(uint64_t bit) const
        {
            const uint64_t b = bit;
            auto it = lower_bound(m_vP.begin(), m_vP.end(), b);
            return (it != m_vP.end() && *it == b) ? 1 : 0;
        }

        // XOR = symmetric difference
        vLbit &operator^=(const vLbit &v)
        {
            vector<uint64_t> out;
            out.reserve(m_vP.size() + v.m_vP.size());

            size_t i = 0, j = 0;
            while (i < m_vP.size() && j < v.m_vP.size())
            {
                if (m_vP[i] < v.m_vP[j])
                {
                    out.push_back(m_vP[i++]);
                }
                else if (v.m_vP[j] < m_vP[i])
                {
                    out.push_back(v.m_vP[j++]);
                }
                else
                {
                    ++i;
                    ++j;
                }
            }

            while (i < m_vP.size())
                out.push_back(m_vP[i++]);
            while (j < v.m_vP.size())
                out.push_back(v.m_vP[j++]);

            m_vP.swap(out);
            return *this;
        }

        // AND = intersection
        vLbit &operator&=(const vLbit &v)
        {
            vector<uint64_t> out;
            out.reserve((m_vP.size() < v.m_vP.size()) ? m_vP.size() : v.m_vP.size());

            size_t i = 0, j = 0;
            while (i < m_vP.size() && j < v.m_vP.size())
            {
                if (m_vP[i] < v.m_vP[j])
                {
                    ++i;
                }
                else if (v.m_vP[j] < m_vP[i])
                {
                    ++j;
                }
                else
                {
                    out.push_back(m_vP[i]);
                    ++i;
                    ++j;
                }
            }

            m_vP.swap(out);
            return *this;
        }

        // OR = union
        vLbit &operator|=(const vLbit &v)
        {
            vector<uint64_t> out;
            out.reserve(m_vP.size() + v.m_vP.size());

            size_t i = 0, j = 0;
            while (i < m_vP.size() && j < v.m_vP.size())
            {
                if (m_vP[i] < v.m_vP[j])
                {
                    out.push_back(m_vP[i++]);
                }
                else if (v.m_vP[j] < m_vP[i])
                {
                    out.push_back(v.m_vP[j++]);
                }
                else
                {
                    out.push_back(m_vP[i]);
                    ++i;
                    ++j;
                }
            }

            while (i < m_vP.size())
                out.push_back(m_vP[i++]);
            while (j < v.m_vP.size())
                out.push_back(v.m_vP[j++]);

            m_vP.swap(out);
            return *this;
        }

        friend vLbit operator^(vLbit a, const vLbit &b)
        {
            a ^= b;
            return a;
        }

        friend vLbit operator&(vLbit a, const vLbit &b)
        {
            a &= b;
            return a;
        }

        friend vLbit operator|(vLbit a, const vLbit &b)
        {
            a |= b;
            return a;
        }

        bool operator==(const vLbit &v) const
        {
            return m_vP == v.m_vP;
        }

        bool operator!=(const vLbit &v) const
        {
            return !(*this == v);
        }

        // compare as big unsigned integer: highest set bit wins
        bool operator<(const vLbit &v) const
        {
            size_t i = m_vP.size();
            size_t j = v.m_vP.size();

            while (i > 0 || j > 0)
            {
                if (i == 0)
                    return true;
                if (j == 0)
                    return false;

                const uint64_t a = m_vP[i - 1];
                const uint64_t b = v.m_vP[j - 1];

                if (a == b)
                {
                    --i;
                    --j;
                    continue;
                }

                return a < b;
            }

            return false;
        }

        bool operator>(const vLbit &v) const
        {
            return v < *this;
        }

        bool operator<=(const vLbit &v) const
        {
            return !(*this > v);
        }

        bool operator>=(const vLbit &v) const
        {
            return !(*this < v);
        }

        // subset test: all bits in *this are also in v
        bool bSubsetOf(const vLbit &v) const
        {
            size_t i = 0, j = 0;

            while (i < m_vP.size() && j < v.m_vP.size())
            {
                if (m_vP[i] == v.m_vP[j])
                {
                    ++i;
                    ++j;
                }
                else if (m_vP[i] > v.m_vP[j])
                {
                    ++j;
                }
                else
                {
                    return false;
                }
            }

            return i == m_vP.size();
        }

        bool bIntersects(const vLbit &v) const
        {
            size_t i = 0, j = 0;

            while (i < m_vP.size() && j < v.m_vP.size())
            {
                if (m_vP[i] == v.m_vP[j])
                    return true;

                if (m_vP[i] < v.m_vP[j])
                    ++i;
                else
                    ++j;
            }

            return false;
        }

        // shift all set-bit positions upward
        void shiftL(uint64_t k)
        {
            if (k == 0 || m_vP.empty())
                return;

            const uint64_t maxP = numeric_limits<uint64_t>::max() - k;
            size_t w = 0;
            for (size_t r = 0; r < m_vP.size(); ++r)
            {
                if (m_vP[r] <= maxP)
                    m_vP[w++] = m_vP[r] + k;
            }
            m_vP.resize(w);
        }

        // shift all set-bit positions downward, dropping negatives
        void shiftR(uint64_t k)
        {
            if (k == 0 || m_vP.empty())
                return;

            size_t w = 0;
            for (size_t r = 0; r < m_vP.size(); ++r)
            {
                if (m_vP[r] >= k)
                    m_vP[w++] = m_vP[r] - k;
            }
            m_vP.resize(w);
        }

        vLbit &operator<<=(uint64_t k)
        {
            shiftL(k);
            return *this;
        }

        vLbit &operator>>=(uint64_t k)
        {
            shiftR(k);
            return *this;
        }

        friend vLbit operator<<(vLbit a, uint64_t k)
        {
            a.shiftL(k);
            return a;
        }

        friend vLbit operator>>(vLbit a, uint64_t k)
        {
            a.shiftR(k);
            return a;
        }

    private:
        vector<uint64_t> m_vP; // sorted unique positions of bits equal to 1
    };
}

#endif
