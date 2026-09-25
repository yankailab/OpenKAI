/*
 * _SharedMemImg.cpp
 *
 *  Created on: Sept 20, 2022
 *      Author: yankai
 */

#include "_SharedMemImg.h"

namespace kai
{

    _SharedMemImg::_SharedMemImg()
    {
        m_type = vision_SharedMemImg;
    }

    _SharedMemImg::~_SharedMemImg()
    {
    }

    bool _SharedMemImg::init(const json &j)
    {
        IF_F(!_VisionBase::init(j));

        jKv(j, "matType", m_matType);

        return true;
    }

    bool _SharedMemImg::link(const json &j, ModuleMgr *pM)
    {
        IF_F(!this->_VisionBase::link(j, pM));

        string n = "";
        jKv(j, "SharedMem", n);
        m_pSHM = (SharedMem *)(pM->findModule(n));
        NULL_F(m_pSHM);

        return true;
    }

    bool _SharedMemImg::start(void)
    {
        NULL_F(m_pT);
        IF_F(!m_pT->startThread(getUpdate, this));
        return true;
    }

    bool _SharedMemImg::check(void)
    {
        NULL_F(m_pT);
        NULL_F(m_pSHM);
        IF_F(!m_pSHM->open());

        return _VisionBase::check();
    }

    void _SharedMemImg::update(void)
    {
        while (m_pT->bRun())
        {
            if (!m_pSHM->bOpen())
            {
                LOG_E("Cannot open shared memory");
                m_pT->sleepT(NSEC_SEC);
                continue;
            }

            m_pT->autoFPS();

            updateSharedMemImg();
        }
    }

    bool _SharedMemImg::updateSharedMemImg(void)
    {
        IF_F(!check());

        std::lock_guard<std::mutex> lock(m_mutexRGB);
        Mat(m_vSizeRGB.y(),
            m_vSizeRGB.x(),
            m_matType,
            m_pSHM->p()).copyTo(m_mRGB);

        return true;
    }
}
