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
    }

    _PCregistICP::~_PCregistICP()
    {
    }

    bool _PCregistICP::loadConfig(void)
    {
        IF_F(!this->_ModuleBase::loadConfig());
        const json &j = *m_pJ;

        jKv(j, "est", (int &)m_est);
        jKv(j, "thr", m_thr);

        return true;
    }

    bool _PCregistICP::saveConfig(bool bExport)
    {
        if (!_ModuleBase::saveConfig(false))
        {
            return false;
        }

        json &j = *m_pJ;
        j["est"] = static_cast<int>(m_est);
        j["thr"] = m_thr;

        if (!bExport)
        {
            return true;
        }
        return m_pJcfg->saveToFile();
    }

    bool _PCregistICP::link(void)
    {
        IF_F(!this->_ModuleBase::link());
        const json &j = *m_pJ;

        string n;

        n = "";
        jKv(j, "_PointCloudSrc", n);
        m_pSrc = (_PointCloud *)(m_pM->findModule(n));
        IF_Le_F(!m_pSrc, n + ": not found");

        n = "";
        jKv(j, "_PointCloudTgt", n);
        m_pTgt = (_PointCloud *)(m_pM->findModule(n));
        IF_Le_F(!m_pTgt, n + ": not found");

        n = "";
        jKv(j, "_PCtransform", n);
        m_pTf = (_PCtransform *)(m_pM->findModule(n));
        IF_Le_F(!m_pTf, n + ": not found");

        return true;
    }

    bool _PCregistICP::start(void)
    {
        NULL_F(m_pT);
        return m_pT->startThread(getUpdate, this);
    }

    bool _PCregistICP::check(void)
    {
        NULL_F(m_pSrc);
        NULL_F(m_pTgt);
        NULL_F(m_pTf);

        return _ModuleBase::check();
    }

    void _PCregistICP::update(void)
    {
        while (m_pT->bRun())
        {
            m_pT->autoFPS();

            updateRegistration();
        }
    }

    void _PCregistICP::updateRegistration(void)
    {
        IF_(!check());

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
