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
        m_maxDistance = 0.1;
        m_rNormal = 0.2;
        m_maxNNnormal = 30;
        m_rFitness = 1e-6;
        m_rRMSE = 1e-6;
        m_maxIter = 30;
        m_minFit = 0.0;
        m_rVoxel = 0.1;

        m_pPCf = NULL;
        m_pTf = NULL;
    }

    _PCregistCol::~_PCregistCol()
    {
    }

    int _PCregistCol::init(void *pKiss)
    {
        CHECK_(_PCframe::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

        pK->v("rVoxel", &m_rVoxel);
        pK->v("maxDistance", &m_maxDistance);
        pK->v("rNormal", &m_rNormal);
        pK->v("maxNNnormal", &m_maxNNnormal);
        pK->v("rFitness", &m_rFitness);
        pK->v("rRMSE", &m_rRMSE);
        pK->v("maxIter", &m_maxIter);
        pK->v("minFit", &m_minFit);

        return OK_OK;
    }

    int _PCregistCol::link(void)
    {
        CHECK_(this->BASE::link());
        Kiss *pK = (Kiss *)m_pKiss;

        string n;

        n = "";
        pK->v("_PCframe", &n);
        m_pPCf = (_PCframe *)(pK->findModule(n));

        return OK_OK;
    }

    int _PCregistCol::start(void)
    {
        NULL__(m_pT, OK_ERR_NULLPTR); // work in none thread mode

        return m_pT->start(getUpdate, this);
    }

    int _PCregistCol::check(void)
    {
        return BASE::check();
    }

    void _PCregistCol::update(void)
    {
        while (m_pT->bAlive())
        {
            m_pT->autoFPSfrom();

            if (updateRegistration())
            {
                updatePC();
            }

            m_pT->autoFPSto();
        }
    }

    void _PCregistCol::updatePC(void)
    {
        mutexLock();
        m_sPC.swap();
        m_sPC.next()->points_.clear();
        m_sPC.next()->colors_.clear();
        m_sPC.next()->normals_.clear();

        m_sPCvd.swap();
        m_sPCvd.next()->points_.clear();
        m_sPCvd.next()->colors_.clear();
        m_sPCvd.next()->normals_.clear();
        mutexUnlock();
    }

    bool _PCregistCol::updateRegistration(void)
    {
        NULL_F(m_pPCf);

        PointCloud* pPC = m_sPC.next();
        m_pPCf->copyTo(pPC);
        IF_F(pPC->IsEmpty());
		*m_sPCvd.next() = *pPC->VoxelDownSample(m_rVoxel);

		IF_T(m_sPCvd.get()->IsEmpty());

        IF_F(updateRegistration(m_sPCvd.next(), m_sPCvd.get()) < m_minFit);

		m_sPC.next()->Transform(m_RR.transformation_);
		m_sPCvd.next()->Transform(m_RR.transformation_);
        if (m_pTf)
            m_pTf->setTranslationMatrix(m_RR.transformation_);

        return true;
    }

    double _PCregistCol::updateRegistration(PointCloud *pSrc, PointCloud *pTgt, Matrix4d_u *pTresult)
    {
        IF__(check() > 0, -1);
        NULL__(pSrc, -1);
        NULL__(pTgt, -1);
        IF__(pSrc->IsEmpty(), -1);
        IF__(pTgt->IsEmpty(), -1);

        if (pSrc->normals_.empty())
            pSrc->EstimateNormals(KDTreeSearchParamHybrid(m_rNormal, m_maxNNnormal));
        if (pTgt->normals_.empty())
            pTgt->EstimateNormals(KDTreeSearchParamHybrid(m_rNormal, m_maxNNnormal));

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
            pTresult = &m_RR.transformation_;

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
