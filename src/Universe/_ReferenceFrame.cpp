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

    bool _ReferenceFrame::loadConfig(void)
    {
        IF_F(!this->_ModuleBase::loadConfig());
        const json &j = *m_pJ;

        jKv<double>(j, "vPos", m_vPos);
        jKv<double>(j, "vOrt", m_vOrt.coeffs());
        setOrientation(m_vOrt, true);

        return true;
    }

    bool _ReferenceFrame::link(void)
    {
        IF_F(!this->_ModuleBase::link());

        return true;
    }

    bool _ReferenceFrame::saveConfig(bool bExport)
    {
        IF_F(!_ModuleBase::saveConfig(false));

        json &j = *m_pJ;
        j["vPos"] = {m_vPos.x(), m_vPos.y(), m_vPos.z()};
        j["vOrt"] = {m_vOrt.x(), m_vOrt.y(), m_vOrt.z(), m_vOrt.w()};

        IF__(!bExport, true);
        return m_pJcfg->saveToFile();
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

    const Vector3d &_ReferenceFrame::getPos(void)
    {
        return m_vPos;
    }

    void _ReferenceFrame::getPos(double &x, double &y, double &z)
    {
        x = m_vPos.x();
        y = m_vPos.y();
        z = m_vPos.z();
    }

    const Vector3d &_ReferenceFrame::getAngles(void)
    {
        return m_vAngle;
    }

    void _ReferenceFrame::getAngles(double &roll, double &pitch, double &yaw)
    {
        roll = m_vAngle.x();
        pitch = m_vAngle.y();
        yaw = m_vAngle.z();
    }

    const Quaterniond &_ReferenceFrame::getOrientation(void)
    {
        return m_vOrt;
    }

    void _ReferenceFrame::getOrientation(double &x, double &y, double &z, double &w)
    {
        x = m_vOrt.x();
        y = m_vOrt.y();
        z = m_vOrt.z();
        w = m_vOrt.w();
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
