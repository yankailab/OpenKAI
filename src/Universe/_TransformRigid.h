/*
 * _TransformRigid.h
 *
 *  Created on: May 24, 2020
 *      Author: yankai
 */

#ifndef OpenKAI_src_Universe__TransformRigid_H_
#define OpenKAI_src_Universe__TransformRigid_H_

#include "../Base/_ModuleBase.h"
#include "../Utility/util.h"
#include "../Utility/utilTime.h"
#include "../UI/_Console.h"
#include <mutex>

namespace kai
{

    // Rigid transforms form a kinematic tree linked before worker threads start.

    class _TransformRigid : public _ModuleBase
    {
    public:
        _TransformRigid();
        virtual ~_TransformRigid();

        virtual bool loadConfig(void) override;
        virtual bool link(InstanceMgr *pM) override;
        virtual bool start(void) override;
        virtual bool check(void) override;
        virtual void console(void *pConsole) override;

        virtual bool saveConfig(bool bExport) override;

        virtual void setTranslation(const Vector3d &vP);
        virtual void setTranslation(double x, double y, double z);
        virtual void setAngles(const Vector3d &vA);
        virtual void setAngles(double roll, double pitch, double yaw);
        virtual void setRotation(const Quaterniond &vOrt, bool bConvertToEulerAngles = false);
        virtual void setRotation(double x, double y, double z, double w, bool bConvertToEulerAngles = false);

        virtual Vector3d getTranslation(void);
        virtual void getTranslation(double &x, double &y, double &z);
        virtual Vector3d getAngles(void);
        virtual void getAngles(double &roll, double &pitch, double &yaw);
        virtual Quaterniond getRotation(void);
        virtual void getRotation(double &x, double &y, double &z, double &w);

        virtual Isometry3d getPose(void);
        virtual Isometry3f getPosef(void);
        virtual Isometry3d getPoseCombined(void);
        virtual Isometry3f getPoseCombinedf(void);

    protected:
        virtual void updatePose(void);
        virtual void updateEulerAngles(void);
        virtual void updatePoseCombined(void);
        virtual void updateChildNodes(void);

    private:
        bool attachParent(_TransformRigid *pParent);
        virtual void update(void);
        static void *getUpdate(void *This)
        {
            ((_TransformRigid *)This)->update();
            return NULL;
        }

    protected:
        // Getters return locked snapshots; callers never retain mutable worker state.
        std::recursive_mutex m_mtxPose;
        _TransformRigid *m_pTfParent = nullptr; // kinematic tree parent node
        vector<_TransformRigid *> m_vpTf;       // kinematic tree children nodes

        Vector3d m_vT = Vector3d::Zero();           // translation
        Quaterniond m_vR = Quaterniond::Identity(); // rotation
        Vector3d m_vAngle = Vector3d::Zero();       // euler angles (radian) in roll, pitch, yaw order

        Isometry3d m_mPose = Isometry3d::Identity();  // combined Translation/Rotation transform for this instance
        Isometry3f m_mPosef = Isometry3f::Identity(); // casted from m_mPose for quick calc

        Isometry3d m_mPoseCombined = Isometry3d::Identity();  // combined transform from parent to this instance
        Isometry3f m_mPoseCombinedf = Isometry3f::Identity(); // casted from m_mPoseCombined for quick calc
    };

}
#endif
