#ifndef OpenKAI_src_Base_vSwitch_H_
#define OpenKAI_src_Base_vSwitch_H_

namespace kai
{

template<typename T>
struct vSwitch
{
    int m_iSwitch;
    T m_pT[2];

    vSwitch(void)
    {
        m_iSwitch = 0;
    }

    void update(void)
    {
    	m_iSwitch = 1 - m_iSwitch;
    }

    T* prev(void)
    {
    	return &m_pT[m_iSwitch];
    }

    T* next(void)
    {
    	return &m_pT[1 - m_iSwitch];
    }
};

}
#endif
