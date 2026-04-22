#ifndef OpenKAI_src_Primitive_vLbit_H_
#define OpenKAI_src_Primitive_vLbit_H_

#include "../Base/platform.h"

using namespace std;

namespace kai
{
    class vLbit
    {
    public:
        static constexpr size_t npos = static_cast<size_t>(-1);

        vLbit() = default;

        // optional convenience ctor
        explicit vLbit(const vector<uint64_t> &vPos)
            : m_vP(vPos)
        {
            normalize();
        }

        explicit vLbit(vector<uint64_t> &&vPos)
            : m_vP(move(vPos))
        {
            normalize();
        }

        // clear all 1-bits
        void clear(void)
        {
            m_vP.clear();
        }

        size_t popcount() const
        {
            return m_vP.size();
        }

        const uint64_t *raw() const
        {
            return m_vP.empty() ? nullptr : m_vP.data();
        }

        const vector<uint64_t>& getVp(void) const
        {
            return m_vP;
        }

        // set bit = 1
        inline void set(size_t bit)
        {
            const uint64_t b = static_cast<uint64_t>(bit);
            auto it = lower_bound(m_vP.begin(), m_vP.end(), b);
            if (it == m_vP.end() || *it != b)
                m_vP.insert(it, b);
        }

        // clear bit = 0
        inline void unset(size_t bit)
        {
            const uint64_t b = static_cast<uint64_t>(bit);
            auto it = lower_bound(m_vP.begin(), m_vP.end(), b);
            if (it != m_vP.end() && *it == b)
                m_vP.erase(it);
        }

        // return 1 if set, else 0
        inline int8_t get(size_t bit) const
        {
            const uint64_t b = static_cast<uint64_t>(bit);
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
        void shiftL(size_t k)
        {
            if (k == 0 || m_vP.empty())
                return;

            const uint64_t kk = static_cast<uint64_t>(k);
            for (auto &p : m_vP)
                p += kk;
        }

        // shift all set-bit positions downward, dropping negatives
        void shiftR(size_t k)
        {
            if (k == 0 || m_vP.empty())
                return;

            const uint64_t kk = static_cast<uint64_t>(k);

            size_t w = 0;
            for (size_t r = 0; r < m_vP.size(); ++r)
            {
                if (m_vP[r] >= kk)
                    m_vP[w++] = m_vP[r] - kk;
            }
            m_vP.resize(w);
        }

        vLbit &operator<<=(size_t k)
        {
            shiftL(k);
            return *this;
        }

        vLbit &operator>>=(size_t k)
        {
            shiftR(k);
            return *this;
        }

        friend vLbit operator<<(vLbit a, size_t k)
        {
            a.shiftL(k);
            return a;
        }

        friend vLbit operator>>(vLbit a, size_t k)
        {
            a.shiftR(k);
            return a;
        }

        // return npos if not found
        size_t findFirstOne(void) const
        {
            return m_vP.empty() ? npos : static_cast<size_t>(m_vP.front());
        }

        // return first set bit strictly after pos, or npos
        size_t findNextOne(size_t pos) const
        {
            auto it = upper_bound(m_vP.begin(), m_vP.end(), static_cast<uint64_t>(pos));
            return (it == m_vP.end()) ? npos : static_cast<size_t>(*it);
        }

        size_t findLastOne() const
        {
            return m_vP.empty() ? npos : static_cast<size_t>(m_vP.back());
        }

        static void xorReduce(vLbit &out, const vLbit *arr, size_t count)
        {
            out.clear();
            if (count == 0 || arr == nullptr)
                return;

            for (size_t i = 0; i < count; ++i)
                out ^= arr[i];
        }

        static void xorReduceInplace(vLbit &out, const vLbit *arr, size_t count)
        {
            if (count == 0 || arr == nullptr)
                return;

            for (size_t i = 0; i < count; ++i)
                out ^= arr[i];
        }

    private:
        void normalize()
        {
            sort(m_vP.begin(), m_vP.end());
            m_vP.erase(unique(m_vP.begin(), m_vP.end()), m_vP.end());
        }

    private:
        vector<uint64_t> m_vP; // sorted unique positions of bits equal to 1
    };
}

#endif