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

        m_vColorDefault.set(1);
        m_vkColR.set(0, 10);
        m_vkColG.set(5, 15);
        m_vkColB.set(10, 20);
        m_bColOverwrite = false;
        m_vkColOv.set(
            1.0 / m_vkColR.len(),
            1.0 / m_vkColG.len(),
            1.0 / m_vkColB.len());

        m_vT.set(0);
        m_vR.set(0);

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

        jKv<float>(j, "vColorDefault", m_vColorDefault);
        jKv<float>(j, "vkColR", m_vkColR);
        jKv<float>(j, "vkColG", m_vkColG);
        jKv<float>(j, "vkColB", m_vkColB);
        jKv(j, "bColOverwrite", m_bColOverwrite);
        m_vkColOv.set(
            1.0 / m_vkColR.len(),
            1.0 / m_vkColG.len(),
            1.0 / m_vkColB.len());

        jKv<double>(j, "vT", m_vT);
        jKv<double>(j, "vR", m_vR);
        setTranslation(m_vT);
        setRotation(m_vR);
        updateTranslationMatrix();

        jKv(j, "fConfig", m_fConfig);
        if (!m_fConfig.empty())
        {
            json jG;
            loadConfig(m_fConfig, jG);
        }

        return true;
    }

    bool _GeometryBase::link(const json &j, ModuleMgr *pM)
    {
        IF_F(!this->_ModuleBase::link(j, pM));

        string n = "";
        jKv(j, "SharedMem", n);
        m_pSM = (SharedMem *)(pM->findModule(n));

        return true;
    }

    string _GeometryBase::getConfigFileName(void)
    {
        return m_fConfig;
    }

    bool _GeometryBase::loadConfig(const string &fName, json &j)
    {
        JsonCfg jCfg;
        IF_Le_F(!jCfg.parseJsonFile(fName), "JSON file parse failed: " + fName);

        j = jCfg.getJson();

        const json &jG = jCfg.getJson("_GeometryBase");
        if (jG.is_object())
        {
            vDouble3 vT, vR;
            vT.clear();
            vR.clear();
            jKv<double>(jG, "vT", vT);
            jKv<double>(jG, "vR", vR);

            setTranslation(vT);
            setRotation(vR);
            updateTranslationMatrix(false);

            vector<double> vmT;
            jKv(jG, "mT", vmT);
            if (vmT.size() >= 16)
            {
                Matrix4d mT;
                mT(0, 0) = vmT[0];
                mT(0, 1) = vmT[1];
                mT(0, 2) = vmT[2];
                mT(0, 3) = vmT[3];

                mT(1, 0) = vmT[4];
                mT(1, 1) = vmT[5];
                mT(1, 2) = vmT[6];
                mT(1, 3) = vmT[7];

                mT(2, 0) = vmT[8];
                mT(2, 1) = vmT[9];
                mT(2, 2) = vmT[10];
                mT(2, 3) = vmT[11];

                mT(3, 0) = vmT[12];
                mT(3, 1) = vmT[13];
                mT(3, 2) = vmT[14];
                mT(3, 3) = vmT[15];

                setTranslationMatrix(mT);
            }
        }

        return true;
    }

    bool _GeometryBase::saveConfig(const string &fName, json &j)
    {
        json jG = json::object();
        jG["vT"] = {m_vT.x, m_vT.y, m_vT.z};
        jG["vR"] = {m_vR.x, m_vR.y, m_vR.z};

        Matrix4d mT = getTranslationMatrix();
        jG["mT"] = {mT(0, 0), mT(0, 1), mT(0, 2), mT(0, 3),
                    mT(1, 0), mT(1, 1), mT(1, 2), mT(1, 3),
                    mT(2, 0), mT(2, 1), mT(2, 2), mT(2, 3),
                    mT(3, 0), mT(3, 1), mT(3, 2), mT(3, 3)};

        j["_GeometryBase"] = jG;

        string f = j.dump();
        return writeFile(fName, f);
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
