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
        m_type = vision_sharedMemImg;
        m_matType = CV_8UC3;
    }

    _SharedMemImg::~_SharedMemImg()
    {
    }

    bool _SharedMemImg::init(void *pKiss)
    {
        IF_F(!_VisionBase::init(pKiss));
        Kiss *pK = (Kiss *)pKiss;

        pK->v("matType", &m_matType);

        return true;
    }

    bool _SharedMemImg::link(void)
    {
        Kiss *pK = (Kiss *)m_pKiss;

        string n;
        n = "";
        F_ERROR_F(pK->v("_SharedMem", &n));
        m_pSHM = (_SharedMem *)(pK->getInst(n));
        NULL_Fl(m_pSHM, "_SharedMem not found");

        return true;
    }

    bool _SharedMemImg::start(void)
    {
        NULL_F(m_pT);
        IF_F(!m_pT->start(getUpdate, this));
    }

    int _SharedMemImg::check(void)
    {
        NULL__(m_pT, -1);
        NULL__(m_pSHM, -1);
        IF__(!m_pSHM->open(), -1);

        return _VisionBase::check();
    }

    void _SharedMemImg::update(void)
    {
        while (m_pT->bRun())
        {
            if (!m_pSHM->bOpen())
            {
                LOG_E("Cannot open shared memory");
                m_pT->sleepT(SEC_2_USEC);
                continue;
            }

            m_pT->autoFPSfrom();

            updateSharedMemImg();

            m_pT->autoFPSto();
        }
    }

    bool _SharedMemImg::updateSharedMemImg(void)
    {
        IF_T(check() < 0);

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
