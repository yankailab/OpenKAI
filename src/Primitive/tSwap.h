#ifndef OpenKAI_src_Primitive_tSwap_H_
#define OpenKAI_src_Primitive_tSwap_H_

namespace kai
{

template<typename T>
struct tSwap
{
    int m_iT = 0;
    T m_pT[2];

    void swap(void)
    {
    	m_iT = 1 - m_iT;
    }

    T* get(void)
    {
    	return &m_pT[m_iT];
    }

    T* next(void)
    {
    	return &m_pT[1 - m_iT];
    }
};

}
#endif
