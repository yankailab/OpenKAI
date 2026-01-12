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

    bool _PCregistICP::init(const json &j)
    {
        IF_F(!this->_ModuleBase::init(j));

        m_est = (PCREGIST_ICP_EST)j.value<int>("est", icp_p2point);
        m_thr = j.value("thr", 0.02);

        return true;
    }

    bool _PCregistICP::link(const json &j, ModuleMgr *pM)
    {
        IF_F(!this->_ModuleBase::link(j, pM));

        string n;

        n = j.value("_PCframeSrc", "");
        m_pSrc = (_PCframe *)(pM->findModule(n));
        if (!m_pSrc)
        {
            LOG_E(n + ": not found");
            return false;
        }

        n = j.value("_PCframeTgt", "");
        m_pTgt = (_PCframe *)(pM->findModule(n));
        if (!m_pTgt)
        {
            LOG_E(n + ": not found");
            return false;
        }

        n = j.value("_PCtransform", "");
        m_pTf = (_PCtransform *)(pM->findModule(n));
        if (!m_pTf)
        {
            LOG_E(n + ": not found");
            return false;
        }

        return true;
    }



    int _PCregistICP::init(void *pKiss)
    {
        CHECK_(_ModuleBase::init(pKiss));
        Kiss *pK = (Kiss *)pKiss;

        pK->v("est", (int *)&m_est);
        pK->v("thr", &m_thr);

        string n;

        n = "";
        pK->v("_PCframeSrc", &n);
        m_pSrc = (_PCframe *)(pK->findModule(n));
        if (!m_pSrc)
        {
            LOG_E(n + ": not found");
            return OK_ERR_NOT_FOUND;
        }

        n = "";
        pK->v("_PCframeTgt", &n);
        m_pTgt = (_PCframe *)(pK->findModule(n));
        if (!m_pTgt)
        {
            LOG_E(n + ": not found");
            return OK_ERR_NOT_FOUND;
        }

        n = "";
        pK->v("_PCtransform", &n);
        m_pTf = (_PCtransform *)(pK->findModule(n));
        if (!m_pTf)
        {
            LOG_E(n + ": not found");
            return OK_ERR_NOT_FOUND;
        }

        return OK_OK;
    }

    int _PCregistICP::start(void)
    {
        NULL__(m_pT, OK_ERR_NULLPTR);
        return m_pT->start(getUpdate, this);
    }

    int _PCregistICP::check(void)
    {
        NULL__(m_pSrc, OK_ERR_NULLPTR);
        NULL__(m_pTgt, OK_ERR_NULLPTR);
        NULL__(m_pTf, OK_ERR_NULLPTR);

        return _ModuleBase::check();
    }

    void _PCregistICP::update(void)
    {
        while (m_pT->bAlive())
        {
            m_pT->autoFPS();

            updateRegistration();
        }
    }

    void _PCregistICP::updateRegistration(void)
    {
        IF_(check() != OK_OK);

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
