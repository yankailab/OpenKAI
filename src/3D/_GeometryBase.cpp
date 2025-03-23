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
        m_fConfig = "";

        m_vColorDefault.set(1.0);
        m_vkColR.set(0, 10.0);
        m_vkColG.set(5, 15.0);
        m_vkColB.set(10, 20.0);
        m_bColOverwrite = false;

        m_vT.clear();
        m_vR.clear();
        m_vQ.clear();
        m_mT = Matrix4d::Identity();
        m_A = Matrix4d::Identity();

        pthread_mutex_init(&m_mutex, NULL);

        m_pSM = nullptr;
    }

    _GeometryBase::~_GeometryBase()
    {
        pthread_mutex_destroy(&m_mutex);
    }

    int _GeometryBase::init(void *pKiss)
    {
        CHECK_(this->_ModuleBase::init(pKiss));
        Kiss *pK = (Kiss *)pKiss;

        pK->v("fConfig", &m_fConfig);
        pK->v("vColorDefault", &m_vColorDefault);
        pK->v("vkColR", &m_vkColR);
        pK->v("vkColG", &m_vkColG);
        pK->v("vkColB", &m_vkColB);
        pK->v("bColOverwrite", &m_bColOverwrite);
        m_vkColOv.set(
            1.0 / m_vkColR.len(),
            1.0 / m_vkColG.len(),
            1.0 / m_vkColB.len()
        );

        pK->v("vT", &m_vT);
        pK->v("vR", &m_vR);
        setTranslation(m_vT);
        setRotation(m_vR);
        updateTranslationMatrix();

        loadConfig();

        return OK_OK;
    }

    int _GeometryBase::link(void)
    {
        CHECK_(this->_ModuleBase::link());
        Kiss *pK = (Kiss *)m_pKiss;

        string n;
        n = "";
        pK->v("SharedMem", &n);
        m_pSM = (SharedMem *)(pK->findModule(n));

        return OK_OK;
    }

    int _GeometryBase::check(void)
    {
        return this->_ModuleBase::check();
    }

    GEOMETRY_TYPE _GeometryBase::getType(void)
    {
        return m_type;
    }

    int _GeometryBase::initGeometry(void)
    {
        return OK_ERR_UNIMPLEMENTED;
    }

    void _GeometryBase::clear(void)
    {
    }

    void _GeometryBase::setTranslation(const vDouble3 &vT)
    {
        m_vT = vT;
    }

    void _GeometryBase::setRotation(const vDouble3 &vR)
    {
        m_vR = vR;
    }

    void _GeometryBase::setQuaternion(const vDouble4 &vQ)
    {
        m_vQ = vQ;
    }

    void _GeometryBase::updateTranslationMatrix(bool bUseQuaternion, vDouble3 *pRa)
    {
        if (bUseQuaternion)
            m_mT = getTranslationMatrix(m_vT, m_vQ, pRa);
        else
            m_mT = getTranslationMatrix(m_vT, m_vR, pRa);

        m_A = m_mT;
    }

    Matrix4d _GeometryBase::getTranslationMatrix(const vDouble3 &vT, const vDouble3 &vR, vDouble3 *pRa)
    {
        Matrix4d mT = Matrix4d::Identity();
        Vector3d eR(vR.x, vR.y, vR.z);
        mT.block(0, 0, 3, 3) = Geometry3D::GetRotationMatrixFromAxisAngle(eR);
        mT(0, 3) = vT.x;
        mT(1, 3) = vT.y;
        mT(2, 3) = vT.z;

        NULL__(pRa, mT);

        eR = Vector3d(pRa->x, pRa->y, pRa->z);
        Matrix3d mR = Geometry3D::GetRotationMatrixFromAxisAngle(eR);
        Matrix3d mTr = mT.block(0, 0, 3, 3);
        mT.block(0, 0, 3, 3) = mTr * mR;

        return mT;
    }

    Matrix4d _GeometryBase::getTranslationMatrix(const vDouble3 &vT, const vDouble4 &vQ, vDouble3 *pRa)
    {
        Matrix4d mT = Matrix4d::Identity();
        Vector4d eQ(vQ.x, vQ.y, vQ.z, vQ.w);
        mT.block(0, 0, 3, 3) = Geometry3D::GetRotationMatrixFromQuaternion(eQ);
        mT(0, 3) = vT.x;
        mT(1, 3) = vT.y;
        mT(2, 3) = vT.z;

        NULL__(pRa, mT);

        Vector3d eR(pRa->x, pRa->y, pRa->z);
        Matrix3d mR = Geometry3D::GetRotationMatrixFromAxisAngle(eR);
        Matrix3d mTr = mT.block(0, 0, 3, 3);
        mT.block(0, 0, 3, 3) = mR * mTr;

        return mT;
    }

    void _GeometryBase::setTranslationMatrix(const Matrix4d &mT)
    {
        m_mT = mT;
        m_A = m_mT;
    }

    vDouble3 _GeometryBase::getTranslation(void)
    {
        return m_vT;
    }

    vDouble3 _GeometryBase::getRotation(void)
    {
        return m_vR;
    }

    vDouble4 _GeometryBase::getQuaternion(void)
    {
        return m_vQ;
    }

    void _GeometryBase::addGeometry(void *p, const uint64_t &tExpire)
    {
        NULL_(p);

        GEOMETRY_TYPE gt = ((_GeometryBase *)p)->getType();
        if (gt == pc_stream)
            addPCstream(p, tExpire);
        else if (gt == pc_frame)
            addPCframe(p);
        else if (gt == pc_grid)
            addPCgrid(p);
    }

    void _GeometryBase::addPCstream(void *p, const uint64_t &tExpire)
    {
    }

    void _GeometryBase::addPCframe(void *p)
    {
    }

    void _GeometryBase::addPCgrid(void *p)
    {
    }

    void _GeometryBase::writeSharedMem(void)
    {
    }

    void _GeometryBase::readSharedMem(void)
    {
    }

    bool _GeometryBase::save2file(const string& fName)
    {
        return false;
    }

    bool _GeometryBase::loadConfig(void)
    {
		string s;
		if(!readFile(m_fConfig, &s))
		{
            LOG_I("Cannot open: " + m_fConfig);
            return false;
		}

        Kiss *pK = new Kiss();
        if (!pK->parse(s))
        {
            LOG_I("Config parse failed: " + m_fConfig);
            DEL(pK);
            return false;
        }

        Kiss *pKc = pK->root()->child("_GeometryBase");
        if (pKc->empty())
        {
            DEL(pK);
            return false;
        }

        vDouble3 vT, vR;
        vT.clear();
        vR.clear();

        pKc->v("vT", &vT);
        pKc->v("vR", &vR);

        setTranslation(vT);
        setRotation(vR);
        updateTranslationMatrix(false);

        DEL(pK);
        return true;
    }

    bool _GeometryBase::saveConfig(void)
    {
        picojson::object o;
        o.insert(make_pair("name", "_GeometryBase"));

        picojson::array vT;
        vT.push_back(value((double)m_vT.x));
        vT.push_back(value((double)m_vT.y));
        vT.push_back(value((double)m_vT.z));
        o.insert(make_pair("vT", vT));

        picojson::array vR;
        vR.push_back(value((double)m_vR.x));
        vR.push_back(value((double)m_vR.y));
        vR.push_back(value((double)m_vR.z));
        o.insert(make_pair("vR", vR));

        string f = picojson::value(o).serialize();

        return writeFile(m_fConfig, f);
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
        pC->addMsg("vQ = (" + f2str(m_vQ.x) + "," + f2str(m_vQ.y) + ", " + f2str(m_vQ.z) + ", " + f2str(m_vQ.w) + ")");
    }

}
