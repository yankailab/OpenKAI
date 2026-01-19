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
        m_matType = CV_8UC3;
    }

    _SharedMemImg::~_SharedMemImg()
    {
    }

    bool _SharedMemImg::init(const json &j)
    {
        IF_F(!_VisionBase::init(j));

        jVar(j, "matType", m_matType);

        return true;
    }

    bool _SharedMemImg::link(const json &j, ModuleMgr *pM)
    {
        IF_F(!this->_VisionBase::link(j, pM));

        string n = "";
        jVar(j, "SharedMem", n);
        m_pSHM = (SharedMem *)(pM->findModule(n));
        NULL_F(m_pSHM);

        return true;
    }

    bool _SharedMemImg::start(void)
    {
        NULL_F(m_pT);
        IF_F(!m_pT->start(getUpdate, this));
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
        while (m_pT->bAlive())
        {
            if (!m_pSHM->bOpen())
            {
                LOG_E("Cannot open shared memory");
                m_pT->sleepT(SEC_2_USEC);
                continue;
            }

            m_pT->autoFPS();

            update_SharedMemImg();
        }
    }

    bool _SharedMemImg::update_SharedMemImg(void)
    {
        IF_F(!check());

        m_fRGB.copy(Mat(m_vSizeRGB.y,
                        m_vSizeRGB.x,
                        m_matType,
                        m_pSHM->p()));

        // imageMat = cv::Mat(m_vzfDepth.height, m_vzfDepth.width, CV_16UC1, m_vzfDepth.pFrameData);
        // imageMat.convertTo(imageMat, CV_8U, 255.0 / m_slope);
        // applyColorMap(imageMat, imageMat, cv::COLORMAP_RAINBOW);
        //    m_fRGB.copy(imageMat);

        // imageMat = cv::Mat(m_vzfTransformedDepth.height, m_vzfTransformedDepth.width, CV_16UC1, m_vzfTransformedDepth.pFrameData);

        //                imageMat = cv::Mat(m_vzfIR.height, m_vzfIR.width, CV_8UC1, m_vzfIR.pFrameData);
        //                m_fRGB.copy(Mat(m_vzfIR.height, m_vzfIR.width, CV_8UC1, m_vzfIR.pFrameData));

        return true;
    }
}
