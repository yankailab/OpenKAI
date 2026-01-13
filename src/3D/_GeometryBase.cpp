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

    bool _GeometryBase::init(const json &j)
    {
        IF_F(!this->_ModuleBase::init(j));

        m_fConfig = j.value("fConfig", "");
        m_vColorDefault = j.value("vColorDefault", vector<float>{1, 1, 1});
        m_vkColR = j.value("vkColR", vector<float>{0, 10});
        m_vkColG = j.value("vkColG", vector<float>{5, 15});
        m_vkColB = j.value("vkColB", vector<float>{10, 20});
        m_bColOverwrite = j.value("bColOverwrite", false);
        m_vkColOv.set(
            1.0 / m_vkColR.len(),
            1.0 / m_vkColG.len(),
            1.0 / m_vkColB.len());

        m_vT = j.value("vT", 0);
        m_vR = j.value("vR", 0);
        setTranslation(m_vT);
        setRotation(m_vR);
        updateTranslationMatrix();

        loadConfig();

        return true;
    }

    bool _GeometryBase::link(const json &j, ModuleMgr *pM)
    {
        IF_F(!this->_ModuleBase::link(j, pM));

        string n = j.value("SharedMem", "");
        m_pSM = (SharedMem *)(pM->findModule(n));

        return true;
    }

    bool _GeometryBase::loadConfig(void)
    {
        string s;
        if (!readFile(m_fConfig, &s))
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

        // vDouble3 vT, vR;
        // vT.clear();
        // vR.clear();

        // pKc->v("vT", &vT);
        // pKc->v("vR", &vR);

        // setTranslation(vT);
        // setRotation(vR);
        // updateTranslationMatrix(false);

        vector<double> vT;
        pKc->a("mT", &vT);

        Matrix4d mT;
        mT(0, 0) = vT[0];
        mT(0, 1) = vT[1];
        mT(0, 2) = vT[2];
        mT(0, 3) = vT[3];

        mT(1, 0) = vT[4];
        mT(1, 1) = vT[5];
        mT(1, 2) = vT[6];
        mT(1, 3) = vT[7];

        mT(2, 0) = vT[8];
        mT(2, 1) = vT[9];
        mT(2, 2) = vT[10];
        mT(2, 3) = vT[11];

        mT(3, 0) = vT[12];
        mT(3, 1) = vT[13];
        mT(3, 2) = vT[14];
        mT(3, 3) = vT[15];

        setTranslationMatrix(mT);

        DEL(pK);
        return true;
    }

    bool _GeometryBase::saveConfig(void)
    {
        picojson::object o;
        o.insert(make_pair("name", "_GeometryBase"));

        // picojson::array vT;
        // vT.push_back(value((double)m_vT.x));
        // vT.push_back(value((double)m_vT.y));
        // vT.push_back(value((double)m_vT.z));
        // o.insert(make_pair("vT", vT));

        // picojson::array vR;
        // vR.push_back(value((double)m_vR.x));
        // vR.push_back(value((double)m_vR.y));
        // vR.push_back(value((double)m_vR.z));
        // o.insert(make_pair("vR", vR));

        Matrix4d mT = getTranslationMatrix();

        picojson::array vT;
        vT.push_back(value((double)mT(0, 0)));
        vT.push_back(value((double)mT(0, 1)));
        vT.push_back(value((double)mT(0, 2)));
        vT.push_back(value((double)mT(0, 3)));

        vT.push_back(value((double)mT(1, 0)));
        vT.push_back(value((double)mT(1, 1)));
        vT.push_back(value((double)mT(1, 2)));
        vT.push_back(value((double)mT(1, 3)));

        vT.push_back(value((double)mT(2, 0)));
        vT.push_back(value((double)mT(2, 1)));
        vT.push_back(value((double)mT(2, 2)));
        vT.push_back(value((double)mT(2, 3)));

        vT.push_back(value((double)mT(3, 0)));
        vT.push_back(value((double)mT(3, 1)));
        vT.push_back(value((double)mT(3, 2)));
        vT.push_back(value((double)mT(3, 3)));

        o.insert(make_pair("mT", vT));

        string f = picojson::value(o).serialize();

        return writeFile(m_fConfig, f);
    }

    bool _GeometryBase::check(void)
    {
        return this->_ModuleBase::check();
    }

    GEOMETRY_TYPE _GeometryBase::getType(void)
    {
        return m_type;
    }

    bool _GeometryBase::initGeometry(void)
    {
        return false;
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
            m_mT = createTranslationMatrix(m_vT, m_vQ, pRa);
        else
            m_mT = createTranslationMatrix(m_vT, m_vR, pRa);

        m_A = m_mT;
    }

    Matrix4d _GeometryBase::createTranslationMatrix(const vDouble3 &vT, const vDouble3 &vR, vDouble3 *pRa)
    {
        Matrix4d mT = Matrix4d::Identity();
        Vector3d eR(vR.x, vR.y, vR.z);
        //        mT.block(0, 0, 3, 3) = Geometry3D::GetRotationMatrixFromAxisAngle(eR);
        mT.block(0, 0, 3, 3) = Geometry3D::GetRotationMatrixFromXYZ(eR);
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

    Matrix4d _GeometryBase::createTranslationMatrix(const vDouble3 &vT, const vDouble4 &vQ, vDouble3 *pRa)
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

    Matrix4d _GeometryBase::getTranslationMatrix(void)
    {
        return m_mT;
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

    bool _GeometryBase::save2file(const string &fName)
    {
        return false;
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
