#include "_AgileX.h"

namespace kai
{

    _AgileX::_AgileX()
    {
        m_pIO = NULL;
        m_iFrame = 0;

        m_spd = 0.0;
        m_steer = 0.0;
    }

    _AgileX::~_AgileX()
    {
    }

    bool _AgileX::init(void *pKiss)
    {
        IF_F(!this->_ModuleBase::init(pKiss));
        Kiss *pK = (Kiss *)pKiss;

        pK->v("spd", &m_spd);
        pK->v("steer", &m_steer);

        string n = "";
        pK->v("_IOBase", &n);
        m_pIO = (_IOBase *)(pK->getInst(n));

        return true;
    }

    int _AgileX::check(void)
    {
        NULL__(m_pIO, -1);

        return this->_ModuleBase::check();
    }

    bool _AgileX::start(void)
    {
        NULL_F(m_pT);
        return m_pT->start(getUpdate, this);
    }

    void _AgileX::update(void)
    {
        while (m_pT->bRun())
        {
            m_pT->autoFPSfrom();

            updateMove();

            m_pT->autoFPSto();
        }
    }

    void _AgileX::updateMove(void)
    {
        IF_(check() < 0);

        move(m_spd, m_steer);
    }

    void _AgileX::move(float spd, float steer)
    {
        uint8_t pC[128];

        pC[0] = 0x5a; //SOF
        pC[1] = 0xa5; //SOF
        pC[2] = 0x0a; //frame_L
        pC[3] = 0x55; //cmd_type
        pC[4] = 0x01; //cmd_id

        pC[5] = 0x02; //serial ctrl
        pC[6] = 0;//clear flag
        pC[7] = 10;
        pC[8] = 0;
        pC[9] = 0;
        pC[10] = 0;
        pC[11] = m_iFrame; //frame_id
        pC[12] = checksum(pC, 12); //check_sum

        m_pIO->write(pC, 13);

        m_iFrame++;
        if(m_iFrame >= 255)m_iFrame = 0;
    }

	void _AgileX::setMode(uint8_t iMode)
    {
        //0: standby, 2: serial ctrl

        uint8_t pC[128];

        pC[0] = 0x5A; //SOF
        pC[1] = 0xA5; //SOF
        pC[2] = 0x05; //frame_L
        pC[3] = 0x55; //cmd_type
        pC[4] = 0x02; //cmd_id
        pC[5] = iMode; //mode
        pC[6] = 0; //frame_id
        pC[7] = checksum(pC, 7); //check_sum

        m_pIO->write(pC, 8);
    }

    /*
    * @brief serial message checksum example code
    * @param[in] *data : serial message data struct pointer
    * @param[in] len :serial message data length
    * @return the checksum result
    */
    uint8_t _AgileX::checksum(uint8_t *data, uint8_t len)
    {
        uint8_t checksum = 0x00;
        for (uint8_t i = 0; i < len; i++)
        {
            checksum += data[i];
        }
        return checksum;
    }

}
