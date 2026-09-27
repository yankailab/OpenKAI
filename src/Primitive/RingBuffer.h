#ifndef OpenKAI_src_Primitive_RingBuffer_H_
#define OpenKAI_src_Primitive_RingBuffer_H_

namespace kai
{

    template <typename T>
    struct RingBuffer
    {
        T *m_pT = nullptr;
        int m_nT = 0;
        int m_iT = 0;

        bool alloc(int nP)
        {
            IF_F(nP <= 0);

            m_pT = new T[nP];
            NULL_F(m_pT);

            m_nT = nP;
            return true;
        }

        void release(void)
        {
            delete[] m_pT;
            m_pT = nullptr;

            m_nT = 0;
            m_iT = 0;
        }

        void clear(void)
        {
            NULL_(m_pT);
            IF_(m_nT <= 0);

            m_iT = 0;
            for (int i = 0; i < m_nT; i++)
                m_pT[i].clear();
        }

        void add(const T &p)
        {
            NULL_(m_pT);
            IF_(m_nT <= m_iT);

            m_pT[m_iT] = p;
            iInc();
        }

        void iInc(void)
        {
            if (++m_iT >= m_nT)
                m_iT = 0;
        }

        int iLastT(void)
        {
            IF__(m_iT <= 0, 0);
            return m_iT - 1;
        }

        int nT(void)
        {
            return m_nT;
        }

        int iDec(int i)
        {
            if (--i < 0)
                i = m_nT - 1;

            return i;
        }

        T *get(int i)
        {
            NULL_N(m_pT);
            IF_N(m_nT <= i);

            return &m_pT[i];
        }
    };

}
#endif
