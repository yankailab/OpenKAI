/*
 * _TransformBase.cpp
 *
 *  Created on: May 24, 2020
 *      Author: yankai
 */

#include "_TransformBase.h"

namespace kai
{

    _TransformBase::_TransformBase()
    {
    }

    _TransformBase::~_TransformBase()
    {
    }

    bool _TransformBase::loadConfig(void)
    {
        IF_F(!this->_ModuleBase::loadConfig());
        const json &j = *m_pJ;

        jKv<double>(j, "vT", m_vT);
        jKv<double>(j, "vOrt", m_vR.coeffs());
        setOrientation(m_vR, true);

        return true;
    }

    bool _TransformBase::link(InstanceMgr *pM)
    {
        IF_F(!this->_ModuleBase::link(pM));

        const json &j = *m_pJ;
        vector<string> vN;
        jKv(j, "_TransformBase", vN);

        m_vpTf.clear();
        m_vpTf.reserve(vN.size());
        for (const string &n : vN)
        {
            _TransformBase *pTf = dynamic_cast<_TransformBase *>(static_cast<_ModuleBase *>(pM->findModule(n)));
            IF_Le_F(!pTf, "_TransformBase not found: " + n);
            m_vpTf.push_back(pTf);
        }

        return true;
    }

    bool _TransformBase::saveConfig(bool bExport)
    {
        IF_F(!_ModuleBase::saveConfig(false));

        json &j = *m_pJ;
        j["vPos"] = {m_vT.x(), m_vT.y(), m_vT.z()};
        j["vOrt"] = {m_vR.x(), m_vR.y(), m_vR.z(), m_vR.w()};

        IF__(!bExport, true);
        return m_pJcfg->saveToFile();
    }

    bool _TransformBase::check(void)
    {
        return this->_ModuleBase::check();
    }

    void _TransformBase::setPos(const Vector3d &vP)
    {
        m_vT = vP;
        updatePose();
    }

    void _TransformBase::setPos(double x, double y, double z)
    {
        m_vT = Vector3d(x, y, z);
        updatePose();
    }

    void _TransformBase::setAngles(const Vector3d &vA)
    {
        setAngles(vA.x(), vA.y(), vA.z());
    }

    void _TransformBase::setAngles(double roll, double pitch, double yaw)
    {
        m_vAngle = Vector3d(roll, pitch, yaw);
        setOrientation(Eigen::AngleAxisd(yaw, Vector3d::UnitZ()) *
                       Eigen::AngleAxisd(pitch, Vector3d::UnitY()) *
                       Eigen::AngleAxisd(roll, Vector3d::UnitX()));
    }

    void _TransformBase::setOrientation(const Quaterniond &vOrt, bool bConvertToEulerAngles)
    {
        m_vR = vOrt.normalized();
        updatePose();

        IF_(!bConvertToEulerAngles);
        updateEulerAngles();
    }

    void _TransformBase::setOrientation(double x, double y, double z, double w, bool bConvertToEulerAngles)
    {
        m_vR = Quaterniond(w, x, y, z).normalized();
        updatePose();

        IF_(!bConvertToEulerAngles);
        updateEulerAngles();
    }

    const Vector3d &_TransformBase::getPos(void)
    {
        return m_vT;
    }

    void _TransformBase::getPos(double &x, double &y, double &z)
    {
        x = m_vT.x();
        y = m_vT.y();
        z = m_vT.z();
    }

    const Vector3d &_TransformBase::getAngles(void)
    {
        return m_vAngle;
    }

    void _TransformBase::getAngles(double &roll, double &pitch, double &yaw)
    {
        roll = m_vAngle.x();
        pitch = m_vAngle.y();
        yaw = m_vAngle.z();
    }

    const Quaterniond &_TransformBase::getOrientation(void)
    {
        return m_vR;
    }

    void _TransformBase::getOrientation(double &x, double &y, double &z, double &w)
    {
        x = m_vR.x();
        y = m_vR.y();
        z = m_vR.z();
        w = m_vR.w();
    }

    void _TransformBase::updatePose(void)
    {
        m_mPose.linear() = m_vR.toRotationMatrix();
        m_mPose.translation() = m_vT;
        m_mPosef = m_mPose.cast<float>();
    }

    void _TransformBase::updateEulerAngles(void)
    {
        m_vAngle = m_vR.toRotationMatrix().canonicalEulerAngles(2, 1, 0).reverse();
    }

    void _TransformBase::console(void *pConsole)
    {
        NULL_(pConsole);
        this->_ModuleBase::console(pConsole);

        _Console *pC = (_Console *)pConsole;
        pC->addMsg("vPos = (" + f2str(m_vT.x()) + "," + f2str(m_vT.y()) + ", " + f2str(m_vT.z()) + ")");
        pC->addMsg("vAngle(R,P,Y) = (" + f2str(m_vAngle.x()) + "," + f2str(m_vAngle.y()) + ", " + f2str(m_vAngle.z()) + ")");
        pC->addMsg("vOrientation = (" + f2str(m_vR.x()) + "," + f2str(m_vR.y()) + ", " + f2str(m_vR.z()) + ", " + f2str(m_vR.w()) + ")");
    }

}
