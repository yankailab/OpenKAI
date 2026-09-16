/*
 * _ReferenceFrame.cpp
 *
 *  Created on: May 24, 2020
 *      Author: yankai
 */

#include "_ReferenceFrame.h"

namespace kai
{

    _ReferenceFrame::_ReferenceFrame()
    {
    }

    _ReferenceFrame::~_ReferenceFrame()
    {
    }

    bool _ReferenceFrame::init(const json &j)
    {
        IF_F(!this->_ModuleBase::init(j));

        jKv<double>(j, "vPos", m_vPos);
        jKv<double>(j, "vOrt", m_vOrt.coeffs());
        setOrientation(m_vOrt, true);

        return true;
    }

    bool _ReferenceFrame::link(const json &j, ModuleMgr *pM)
    {
        IF_F(!this->_ModuleBase::link(j, pM));

        return true;
    }

    bool _ReferenceFrame::loadConfig(json *pJ, string fName)
    {
        json j;
        IF_F(!this->_ModuleBase::loadConfig(&j, fName));

        const json &jG = jK(j, "_ReferenceFrame");
        if (jG.is_object())
        {
            jKv<double>(jG, "vPos", m_vPos);
            jKv<double>(jG, "vOrt", m_vOrt.coeffs());
            setOrientation(m_vOrt, true);
        }

        if (pJ)
            *pJ = std::move(j);

        return true;
    }

    bool _ReferenceFrame::saveConfig(json &j, string fName)
    {
        j["_ReferenceFrame"] = {
            {"vPos", {m_vPos.x(), m_vPos.y(), m_vPos.z()}},
            {"vOrt", {m_vOrt.x(), m_vOrt.y(), m_vOrt.z(), m_vOrt.w()}}};

        return this->_ModuleBase::saveConfig(j, fName);
    }

    bool _ReferenceFrame::check(void)
    {
        return this->_ModuleBase::check();
    }

    void _ReferenceFrame::setPos(const Vector3d &vP)
    {
        m_vPos = vP;
        updatePose();
    }

    void _ReferenceFrame::setPos(double x, double y, double z)
    {
        m_vPos = Vector3d(x, y, z);
        updatePose();
    }

    void _ReferenceFrame::setAngles(const Vector3d &vA)
    {
        setAngles(vA.x(), vA.y(), vA.z());
    }

    void _ReferenceFrame::setAngles(double roll, double pitch, double yaw)
    {
        m_vAngle = Vector3d(roll, pitch, yaw);
        setOrientation(Eigen::AngleAxisd(yaw, Vector3d::UnitZ()) *
                       Eigen::AngleAxisd(pitch, Vector3d::UnitY()) *
                       Eigen::AngleAxisd(roll, Vector3d::UnitX()));
    }

    void _ReferenceFrame::setOrientation(const Quaterniond &vOrt, bool bConvertToEulerAngles)
    {
        m_vOrt = vOrt.normalized();
        updatePose();

        IF_(!bConvertToEulerAngles);
        updateEulerAngles();
    }

    void _ReferenceFrame::setOrientation(double x, double y, double z, double w, bool bConvertToEulerAngles)
    {
        m_vOrt = Quaterniond(w, x, y, z).normalized();
        updatePose();

        IF_(!bConvertToEulerAngles);
        updateEulerAngles();
    }

    void _ReferenceFrame::updatePose(void)
    {
        m_mPose.linear() = m_vOrt.toRotationMatrix();
        m_mPose.translation() = m_vPos;
        m_mPosef = m_mPose.cast<float>();
    }

    void _ReferenceFrame::updateEulerAngles(void)
    {
        m_vAngle = m_vOrt.toRotationMatrix().canonicalEulerAngles(2, 1, 0).reverse();
    }

    void _ReferenceFrame::console(void *pConsole)
    {
        NULL_(pConsole);
        this->_ModuleBase::console(pConsole);

        _Console *pC = (_Console *)pConsole;
        pC->addMsg("vPos = (" + f2str(m_vPos.x()) + "," + f2str(m_vPos.y()) + ", " + f2str(m_vPos.z()) + ")");
        pC->addMsg("vAngle(R,P,Y) = (" + f2str(m_vAngle.x()) + "," + f2str(m_vAngle.y()) + ", " + f2str(m_vAngle.z()) + ")");
        pC->addMsg("vOrientation = (" + f2str(m_vOrt.x()) + "," + f2str(m_vOrt.y()) + ", " + f2str(m_vOrt.z()) + ", " + f2str(m_vOrt.w()) + ")");
    }

}
