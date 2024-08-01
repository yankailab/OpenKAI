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

    int _SharedMemImg::init(void *pKiss)
    {
        CHECK_(_VisionBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

        pK->v("matType", &m_matType);

        return OK_OK;
    }

    int _SharedMemImg::link(void)
    {
        Kiss *pK = (Kiss *)m_pKiss;

        string n;
        n = "";
        pK->v("SharedMem", &n);
        m_pSHM = (SharedMem *)(pK->findModule(n));
        NULL__(m_pSHM, OK_ERR_NOT_FOUND);

        return OK_OK;
    }

    int _SharedMemImg::start(void)
    {
        NULL__(m_pT, OK_ERR_NULLPTR);
        IF_F(!m_pT->start(getUpdate, this));
    }

    int _SharedMemImg::check(void)
    {
        NULL__(m_pT, OK_ERR_NULLPTR);
        NULL__(m_pSHM, OK_ERR_NULLPTR);
        IF__(!m_pSHM->open(), OK_ERR_NOT_READY);

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

            m_pT->autoFPSfrom();

            update_SharedMemImg();

            m_pT->autoFPSto();
        }
    }

    bool _SharedMemImg::update_SharedMemImg(void)
    {
        IF__(check() != OK_OK, true);

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
