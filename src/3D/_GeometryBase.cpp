/*
 * _GeometryBase.cpp
 *
 *  Created on: May 24, 2020
 *      Author: yankai
 */

#include "_GeometryBase.h"

namespace kai
{

    _GeometryBase::_GeometryBase()
    {
        m_type = geometry_unknown;

        m_vDefaultColor.set(1.0);

        m_vT.clear();
        m_vR.clear();
        m_mT = Matrix4d::Identity();
        m_A = Matrix4d::Identity();

        pthread_mutex_init(&m_mutex, NULL);

        m_pSM = NULL;
    }

    _GeometryBase::~_GeometryBase()
    {
        pthread_mutex_destroy(&m_mutex);
    }

    bool _GeometryBase::init(void *pKiss)
    {
        IF_F(!this->_ModuleBase::init(pKiss));
        Kiss *pK = (Kiss *)pKiss;

        pK->v("vDefaultColor", &m_vDefaultColor);

        pK->v("vT", &m_vT);
        pK->v("vR", &m_vR);
        setTranslation(m_vT);
        setRotation(m_vR);
        updateTranslationMatrix();

        return true;
    }

    bool _GeometryBase::link(void)
    {
        IF_F(!this->_ModuleBase::link());
        Kiss *pK = (Kiss *)m_pKiss;

        string n;
        n = "";
        pK->v("SharedMem", &n);
        m_pSM = (SharedMem *)(pK->getInst(n));

        return true;
    }

    int _GeometryBase::check(void)
    {
        return this->_ModuleBase::check();
    }

    void _GeometryBase::setTranslation(const vDouble3 &vT)
    {
        m_vT = vT;
    }

    void _GeometryBase::setRotation(const vDouble3 &vR)
    {
        m_vR = vR;
    }

    void _GeometryBase::updateTranslationMatrix(void)
    {
        m_mT = getTranslationMatrix(m_vT, m_vR);
        m_A = m_mT;
    }

    void _GeometryBase::setTranslationMatrix(const Matrix4d &mT)
    {
        m_mT = mT;
        m_A = m_mT;
    }

    Matrix4d _GeometryBase::getTranslationMatrix(const vDouble3 &vT, const vDouble3 &vR)
    {
        Matrix4d mT = Matrix4d::Identity();
        Vector3d eR(vR.x, vR.y, vR.z);
        mT.block(0, 0, 3, 3) = Geometry3D::GetRotationMatrixFromXYZ(eR);
        mT(0, 3) = vT.x;
        mT(1, 3) = vT.y;
        mT(2, 3) = vT.z;

        return mT;
    }

    GEOMETRY_TYPE _GeometryBase::getType(void)
    {
        return m_type;
    }

    bool _GeometryBase::initBuffer(void)
    {
        return false;
    }

    void _GeometryBase::clear(void)
    {
    }

    void _GeometryBase::getGeometry(void *p, const uint64_t &tExpire)
    {
        NULL_(p);

        GEOMETRY_TYPE gt = ((_GeometryBase *)p)->getType();
        if (gt == pc_stream)
            getPCstream(p, tExpire);
        else if (gt == pc_frame)
            getPCframe(p);
        else if (gt == pc_grid)
            getPCgrid(p);
    }

    void _GeometryBase::getPCstream(void *p, const uint64_t &tExpire)
    {
    }

    void _GeometryBase::getPCframe(void *p)
    {
    }

    void _GeometryBase::getPCgrid(void *p)
    {
    }

    void _GeometryBase::writeSharedMem(void)
    {
    }

    void _GeometryBase::readSharedMem(void)
    {
    }

    void _GeometryBase::mutexLock(void)
    {
        pthread_mutex_lock(&m_mutex);
    }

    void _GeometryBase::mutexUnlock(void)
    {
        pthread_mutex_unlock(&m_mutex);
    }

    void _GeometryBase::console(void *pConsole)
    {
        NULL_(pConsole);
        this->_ModuleBase::console(pConsole);

        _Console *pC = (_Console *)pConsole;
        pC->addMsg("vT = (" + f2str(m_vT.x) + "," + f2str(m_vT.y) + ", " + f2str(m_vT.z) + ")");
        pC->addMsg("vR = (" + f2str(m_vR.x) + "," + f2str(m_vR.y) + ", " + f2str(m_vR.z) + ")");
    }

}
