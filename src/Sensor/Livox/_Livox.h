/*
 * _Livox.h
 *
 *  Created on: Aug 21, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_Sensor__Livox_H_
#define OpenKAI_src_Sensor__Livox_H_

#ifdef USE_LIVOX
#include "../../PointCloud/_PCstream.h"
#include "LivoxLidar.h"

namespace kai
{

class _Livox: public _PCstream
{
public:
    _Livox();
    ~_Livox();

    virtual bool init ( void* pKiss );
    virtual int check ( void );
    virtual bool start ( void );
    virtual void draw ( void );

protected:
    static void CbRecvData(LivoxEthPacket* pData, void* pLivox);
    
    void addP(LivoxRawPoint* pLp, uint64_t& tStamp);
    void addP(LivoxExtendRawPoint* pLp, uint64_t& tStamp);
    void addDualP(LivoxDualExtendRawPoint* pLp, uint64_t& tStamp);
    void addTripleP(LivoxTripleExtendRawPoint* pLp, uint64_t& tStamp);
    void updateIMU(LivoxImuPoint* pLd);
    
    bool open ( void );
    void close ( void );
    bool updateLidar ( void );
    void update ( void );
    static void* getUpdate ( void* This )
    {
        ( ( _Livox* ) This )->update();
        return NULL;
    }
    
public:
    bool m_bOpen;
    string m_broadcastCode;
    LivoxLidar* m_pL;
    uint32_t m_iTransformed;
    const float m_ovRef = 1.0/255.0;    
};

}
#endif
#endif
