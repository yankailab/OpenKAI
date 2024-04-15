/*
 * _PCregistICP.cpp
 *
 *  Created on: Sept 6, 2020
 *      Author: yankai
 */

#ifdef USE_OPEN3D
#include "_PCregistICP.h"

namespace kai
{

    _PCregistICP::_PCregistICP()
    {
        m_est = icp_p2point;
        m_thr = 0.02;
        m_pSrc = NULL;
        m_pTgt = NULL;
        m_pTf = NULL;
        m_lastFit = 0.0;
    }

    _PCregistICP::~_PCregistICP()
    {
    }

    bool _PCregistICP::init(void *pKiss)
    {
        IF_F(!_ModuleBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

        pK->v("est", (int *)&m_est);
        pK->v("thr", &m_thr);

        string n;

        n = "";
        pK->v("_PCframeSrc", &n);
        m_pSrc = (_PCframe *)(pK->getInst(n));
        IF_Fl(!m_pSrc, n + ": not found");

        n = "";
        pK->v("_PCframeTgt", &n);
        m_pTgt = (_PCframe *)(pK->getInst(n));
        IF_Fl(!m_pTgt, n + ": not found");

        n = "";
        pK->v("_PCtransform", &n);
        m_pTf = (_PCtransform *)(pK->getInst(n));
        IF_Fl(!m_pTf, n + ": not found");

        return true;
    }

    bool _PCregistICP::start(void)
    {
        NULL_F(m_pT);
        return m_pT->start(getUpdate, this);
    }

    int _PCregistICP::check(void)
    {
        NULL__(m_pSrc, -1);
        NULL__(m_pTgt, -1);
        NULL__(m_pTf, -1);

        return _ModuleBase::check();
    }

    void _PCregistICP::update(void)
    {
        while (m_pT->bAlive())
        {
            m_pT->autoFPSfrom();

            updateRegistration();

            m_pT->autoFPSto();
        }
    }

    void _PCregistICP::updateRegistration(void)
    {
        IF_(check() < 0);

        PointCloud pcSrc;
        m_pSrc->copyTo(&pcSrc);
        PointCloud pcTgt;
        m_pTgt->copyTo(&pcTgt);

        IF_(pcSrc.IsEmpty());
        IF_(pcTgt.IsEmpty());

        if (m_est == icp_p2point)
        {
            m_RR = RegistrationICP(
                pcSrc,
                pcTgt,
                m_thr,
                //                Eigen::Matrix4d::Identity(),
                m_RR.transformation_,
                TransformationEstimationPointToPoint());
        }
        else if (m_est == icp_p2plane)
        {
            m_RR = RegistrationICP(
                pcSrc,
                pcTgt,
                m_thr,
                //                Eigen::Matrix4d::Identity(),
                m_RR.transformation_,
                TransformationEstimationPointToPlane());
        }
        else
        {
            return;
        }

        IF_(m_RR.fitness_ < m_lastFit);
        m_lastFit = m_RR.fitness_;

        m_pTf->setTranslationMatrix(m_RR.transformation_);
    }

    void _PCregistICP::console(void *pConsole)
    {
        NULL_(pConsole);
        this->_ModuleBase::console(pConsole);
        ((_Console *)pConsole)->addMsg("Fitness = " + f2str((float)m_RR.fitness_) + ", Inliner_rmse = " + f2str((float)m_RR.inlier_rmse_));
    }

}
#endif
