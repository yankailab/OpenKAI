/*
 * _TransformRigid.cpp
 *
 *  Created on: May 24, 2020
 *      Author: yankai
 */

#include "_TransformRigid.h"

namespace kai
{

    _TransformRigid::_TransformRigid()
    {
    }

    _TransformRigid::~_TransformRigid()
    {
        if (m_pT)
        {
            m_pT->join();
        }
    }

    bool _TransformRigid::loadConfig(void)
    {
        IF_F(!this->_ModuleBase::loadConfig());
        const json &j = *m_pJ;

        jKv<double>(j, "vT", m_vT);
        jKv<double>(j, "vR", m_vR.coeffs());
        IF_Le_F(!m_vT.allFinite() || !m_vR.coeffs().allFinite() || m_vR.norm() <= 0.0 ||
                !std::isfinite(m_vR.norm()), "Invalid rigid transform");
        setRotation(m_vR, true);

        return true;
    }

    bool _TransformRigid::link(InstanceMgr *pM)
    {
        IF_F(!this->_ModuleBase::link(pM));

        const json &j = *m_pJ;

        string nP;
        jKv(j, "_TransformRigidParent", nP);
        _TransformRigid *pParent = dynamic_cast<_TransformRigid *>(static_cast<_ModuleBase *>(pM->findModule(nP)));
        IF_Le_F(!nP.empty() && !pParent, "_TransformRigidParent not found: " + nP);
        if (pParent && !attachParent(pParent))
        {
            return false;
        }

        vector<string> vN;
        jKv(j, "_TransformRigidChildren", vN);
        for (const string &n : vN)
        {
            _TransformRigid *pTf = dynamic_cast<_TransformRigid *>(static_cast<_ModuleBase *>(pM->findModule(n)));
            IF_Le_F(!pTf, "_TransformRigid child not found: " + n);
            IF_F(!pTf->attachParent(this));
        }

        updatePoseCombined();
        return true;
    }

    bool _TransformRigid::attachParent(_TransformRigid *pParent)
    {
        // A node may be named from either end of an edge, but has only one parent.
        IF_Le_F(m_pTfParent && m_pTfParent != pParent, "Conflicting rigid transform parents");
        for (_TransformRigid *pTf = pParent; pTf; pTf = pTf->m_pTfParent)
        {
            IF_Le_F(pTf == this, "Cycle in rigid transform tree");
        }

        m_pTfParent = pParent;
        auto &children = pParent->m_vpTf;
        if (std::find(children.begin(), children.end(), this) == children.end())
        {
            children.push_back(this);
        }
        return true;
    }

    bool _TransformRigid::start(void)
    {
        NULL_F(m_pT);
        return m_pT->startThread(getUpdate, this);
    }

    void _TransformRigid::update(void)
    {
        while (m_pT->bRun())
        {
            m_pT->autoFPS();

            if (!m_pT->bRun())
            {
                break;
            }
            updatePoseCombined();
        }
    }

    bool _TransformRigid::saveConfig(bool bExport)
    {
        IF_F(!_ModuleBase::saveConfig(false));

        std::lock_guard<std::recursive_mutex> lock(m_mtxPose);
        json &j = *m_pJ;
        j["vT"] = {m_vT.x(), m_vT.y(), m_vT.z()};
        j["vR"] = {m_vR.x(), m_vR.y(), m_vR.z(), m_vR.w()};

        IF__(!bExport, true);
        return m_pJcfg->saveToFile();
    }

    bool _TransformRigid::check(void)
    {
        return this->_ModuleBase::check();
    }

    void _TransformRigid::setTranslation(const Vector3d &vP)
    {
        std::lock_guard<std::recursive_mutex> lock(m_mtxPose);
        if (!vP.allFinite())
        {
            return;
        }
        m_vT = vP;
        updatePose();
    }

    void _TransformRigid::setTranslation(double x, double y, double z)
    {
        setTranslation(Vector3d(x, y, z));
    }

    void _TransformRigid::setAngles(const Vector3d &vA)
    {
        setAngles(vA.x(), vA.y(), vA.z());
    }

    void _TransformRigid::setAngles(double roll, double pitch, double yaw)
    {
        std::lock_guard<std::recursive_mutex> lock(m_mtxPose);
        if (!Vector3d(roll, pitch, yaw).allFinite())
        {
            return;
        }
        m_vAngle = Vector3d(roll, pitch, yaw);
        setRotation(Eigen::AngleAxisd(yaw, Vector3d::UnitZ()) *
                    Eigen::AngleAxisd(pitch, Vector3d::UnitY()) *
                    Eigen::AngleAxisd(roll, Vector3d::UnitX()));
    }

    void _TransformRigid::setRotation(const Quaterniond &vOrt, bool bConvertToEulerAngles)
    {
        std::lock_guard<std::recursive_mutex> lock(m_mtxPose);
        if (!vOrt.coeffs().allFinite() || vOrt.norm() <= 0.0 || !std::isfinite(vOrt.norm()))
        {
            return;
        }
        m_vR = vOrt.normalized();
        updatePose();

        IF_(!bConvertToEulerAngles);
        updateEulerAngles();
    }

    void _TransformRigid::setRotation(double x, double y, double z, double w, bool bConvertToEulerAngles)
    {
        setRotation(Quaterniond(w, x, y, z), bConvertToEulerAngles);
    }

    Vector3d _TransformRigid::getTranslation(void)
    {
        std::lock_guard<std::recursive_mutex> lock(m_mtxPose);
        return m_vT;
    }

    void _TransformRigid::getTranslation(double &x, double &y, double &z)
    {
        std::lock_guard<std::recursive_mutex> lock(m_mtxPose);
        x = m_vT.x();
        y = m_vT.y();
        z = m_vT.z();
    }

    Vector3d _TransformRigid::getAngles(void)
    {
        std::lock_guard<std::recursive_mutex> lock(m_mtxPose);
        return m_vAngle;
    }

    void _TransformRigid::getAngles(double &roll, double &pitch, double &yaw)
    {
        std::lock_guard<std::recursive_mutex> lock(m_mtxPose);
        roll = m_vAngle.x();
        pitch = m_vAngle.y();
        yaw = m_vAngle.z();
    }

    Quaterniond _TransformRigid::getRotation(void)
    {
        std::lock_guard<std::recursive_mutex> lock(m_mtxPose);
        return m_vR;
    }

    void _TransformRigid::getRotation(double &x, double &y, double &z, double &w)
    {
        std::lock_guard<std::recursive_mutex> lock(m_mtxPose);
        x = m_vR.x();
        y = m_vR.y();
        z = m_vR.z();
        w = m_vR.w();
    }

    Isometry3d _TransformRigid::getPose(void)
    {
        std::lock_guard<std::recursive_mutex> lock(m_mtxPose);
        return m_mPose;
    }
    Isometry3f _TransformRigid::getPosef(void)
    {
        std::lock_guard<std::recursive_mutex> lock(m_mtxPose);
        return m_mPosef;
    }

    Isometry3d _TransformRigid::getPoseCombined(void)
    {
        std::lock_guard<std::recursive_mutex> lock(m_mtxPose);
        updatePoseCombined();
        return m_mPoseCombined;
    }

    Isometry3f _TransformRigid::getPoseCombinedf(void)
    {
        std::lock_guard<std::recursive_mutex> lock(m_mtxPose);
        updatePoseCombined();
        return m_mPoseCombinedf;
    }

    void _TransformRigid::updatePose(void)
    {
        m_mPose.linear() = m_vR.toRotationMatrix();
        m_mPose.translation() = m_vT;
        m_mPosef = m_mPose.cast<float>();
        updatePoseCombined();
    }

    void _TransformRigid::updateEulerAngles(void)
    {
        m_vAngle = m_vR.toRotationMatrix().canonicalEulerAngles(2, 1, 0).reverse();
    }

    void _TransformRigid::updatePoseCombined(void)
    {
        std::lock_guard<std::recursive_mutex> lock(m_mtxPose);
        const Isometry3d pose = m_pTfParent ? m_pTfParent->getPoseCombined() * m_mPose : m_mPose;
        if ((pose.matrix().array() == m_mPoseCombined.matrix().array()).all())
        {
            return;
        }
        m_mPoseCombined = pose;
        m_mPoseCombinedf = pose.cast<float>();
        updateChildNodes();
    }

    void _TransformRigid::updateChildNodes(void)
    {
        for (_TransformRigid *pTf : m_vpTf)
        {
            pTf->catchUp();
        }
    }

    void _TransformRigid::console(void *pConsole)
    {
        NULL_(pConsole);
        this->_ModuleBase::console(pConsole);

        std::lock_guard<std::recursive_mutex> lock(m_mtxPose);
        _Console *pC = (_Console *)pConsole;
        pC->addMsg("vT = (" + f2str(m_vT.x()) + "," + f2str(m_vT.y()) + ", " + f2str(m_vT.z()) + ")");
        pC->addMsg("vAngle(R,P,Y) = (" + f2str(m_vAngle.x()) + "," + f2str(m_vAngle.y()) + ", " + f2str(m_vAngle.z()) + ")");
        pC->addMsg("vR = (" + f2str(m_vR.x()) + "," + f2str(m_vR.y()) + ", " + f2str(m_vR.z()) + ", " + f2str(m_vR.w()) + ")");
    }

}
