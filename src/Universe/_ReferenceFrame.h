/*
 * PCbase.h
 *
 *  Created on: May 24, 2020
 *      Author: yankai
 */

#ifndef OpenKAI_src_Universe__ReferenceFrame_H_
#define OpenKAI_src_Universe__ReferenceFrame_H_

#include "../Base/_ModuleBase.h"
#include "../Utility/util.h"
#include "../Utility/utilTime.h"
#include "../UI/_Console.h"

namespace kai
{

    class _ReferenceFrame : public _ModuleBase
    {
    public:
        _ReferenceFrame();
        virtual ~_ReferenceFrame();

        virtual bool init(const json &j);
        virtual bool link(const json &j, ModuleMgr *pM);
        virtual bool check(void);
        virtual void console(void *pConsole);

        virtual bool loadConfig(json *pJ = nullptr, string fName = "");
        virtual bool saveConfig(json &j, string fName = "");

        virtual void setPos(const Vector3d &vP);
        virtual void setPos(double x, double y, double z);
        virtual void setAngles(const Vector3d &vA);
        virtual void setAngles(double roll, double pitch, double yaw);
        virtual void setOrientation(const Quaterniond &vOrt, bool bConvertToEulerAngles = false);
        virtual void setOrientation(double x, double y, double z, double w, bool bConvertToEulerAngles = false);

        virtual const Vector3d &getPos(void);
        virtual void getPos(double &x, double &y, double &z);
        virtual const Vector3d &getAngles(void);
        virtual void getAngles(double &roll, double &pitch, double &yaw);
        virtual const Quaterniond &getOrientation(void);
        virtual void getOrientation(double &x, double &y, double &z, double &w);

    protected:
        virtual void updatePose(void);
        virtual void updateEulerAngles(void);

    protected:
        Vector3d m_vPos = Vector3d::Zero();           // position
        Quaterniond m_vOrt = Quaterniond::Identity(); // orientation quaternion
        Vector3d m_vAngle = Vector3d::Zero();         // euler angles in roll, pitch, yaw order

        Isometry3d m_mPose = Isometry3d::Identity();  // combined transform
        Isometry3f m_mPosef = Isometry3f::Identity(); // casted from m_mPose for quick calc
    };

}
#endif
