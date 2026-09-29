/*
 * _PCregistCol.cpp
 *
 *  Created on: Sept 6, 2020
 *      Author: yankai
 */

#ifdef USE_OPEN3D
#include "_PCregistCol.h"

namespace kai
{

    _PCregistCol::_PCregistCol()
    {
    }

    _PCregistCol::~_PCregistCol()
    {
    }

    bool _PCregistCol::loadConfig(void)
    {
        IF_F(!this->_ReferenceFrame::loadConfig());
        const json &j = *m_pJ;

        jKv(j, "rVoxel", m_rVoxel);
        jKv(j, "maxDistance", m_maxDistance);
        jKv(j, "rNormal", m_rNormal);
        jKv(j, "maxNNnormal", m_maxNNnormal);
        jKv(j, "rFitness", m_rFitness);
        jKv(j, "rRMSE", m_rRMSE);
        jKv(j, "maxIter", m_maxIter);
        jKv(j, "minFit", m_minFit);

        return true;
    }

    bool _PCregistCol::saveConfig(bool bExport)
    {
        IF_F(!_ReferenceFrame::saveConfig(false));

        json &j = *m_pJ;
        j["rVoxel"] = m_rVoxel;
        j["maxDistance"] = m_maxDistance;
        j["rNormal"] = m_rNormal;
        j["maxNNnormal"] = m_maxNNnormal;
        j["rFitness"] = m_rFitness;
        j["rRMSE"] = m_rRMSE;
        j["maxIter"] = m_maxIter;
        j["minFit"] = m_minFit;

        IF__(!bExport, true);
        return m_pJcfg->saveToFile();
    }

    bool _PCregistCol::link(InstanceMgr *pM)
    {
        IF_F(!this->_ReferenceFrame::link(pM));
        const json &j = *m_pJ;

        string n = "";
        jKv(j, "PCLframe", n);
        m_pPCL = dynamic_cast<PCLframe *>(static_cast<DataObjBase *>(pM->findDataObject(n)));
        IF_Le_F(!m_pPCL, "PCLframe not found: " + n);

        n = "";
        jKv(j, "PCLframeIn", n);
        m_pPCLin = dynamic_cast<PCLframe *>(static_cast<DataObjBase *>(pM->findDataObject(n)));
        IF_Le_F(!m_pPCLin, "PCLframeIn not found: " + n);
        IF_Le_F(m_pPCLin == m_pPCL, "PCLframeIn must differ from PCLframe");

        return true;
    }

    bool _PCregistCol::start(void)
    {
        NULL_F(m_pT); // work in none thread mode

        return m_pT->startThread(getUpdate, this);
    }

    bool _PCregistCol::check(void)
    {
        return m_pPCL && m_pPCLin && _ReferenceFrame::check();
    }

    void _PCregistCol::clear(void)
    {
        if (m_pPCL)
        {
            m_pPCL->set({});
        }
    }

    void _PCregistCol::update(void)
    {
        while (m_pT->bRun())
        {
            m_pT->autoFPS();

            if (updateRegistration())
            {
                updatePC();
            }
        }
    }

    void _PCregistCol::updatePC(void)
    {
        // TODO: publish the registered point cloud to m_pPCL.
    }

    bool _PCregistCol::updateRegistration(void)
    {
        IF_F(!check());

        // TODO: register successive snapshots from m_pPCLin.
        return true;
    }

    double _PCregistCol::updateRegistration(open3d::geometry::PointCloud *pSrc, open3d::geometry::PointCloud *pTgt, Eigen::Matrix4d *pTresult)
    {
        IF__(!check(), -1);
        NULL__(pSrc, -1);
        NULL__(pTgt, -1);
        IF__(pSrc->IsEmpty(), -1);
        IF__(pTgt->IsEmpty(), -1);

        if (pSrc->normals_.empty())
        {
            pSrc->EstimateNormals(open3d::geometry::KDTreeSearchParamHybrid(m_rNormal, m_maxNNnormal));
        }
        if (pTgt->normals_.empty())
        {
            pTgt->EstimateNormals(open3d::geometry::KDTreeSearchParamHybrid(m_rNormal, m_maxNNnormal));
        }

        m_RR = RegistrationColoredICP(
            *pSrc,
            *pTgt,
            m_maxDistance,
            m_RR.transformation_,
            TransformationEstimationForColoredICP(),
            ICPConvergenceCriteria(m_rFitness,
                                   m_rRMSE,
                                   m_maxIter));

        if (pTresult)
        {
            *pTresult = m_RR.transformation_;
        }

        return m_RR.fitness_;
    }

    void _PCregistCol::console(void *pConsole)
    {
        NULL_(pConsole);
        this->_ModuleBase::console(pConsole);
        ((_Console *)pConsole)->addMsg("Fitness = " + f2str((float)m_RR.fitness_) + ", Inliner_rmse = " + f2str((float)m_RR.inlier_rmse_));
    }

}
#endif
