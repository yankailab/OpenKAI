/*
 * _PointCloud.h
 *
 *  Created on: May 24, 2020
 *      Author: yankai
 */

#ifndef OpenKAI_src_Universe_Geometry_PointCloud__PointCloud_H_
#define OpenKAI_src_Universe_Geometry_PointCloud__PointCloud_H_

#include "../_GeometryBase.h"
#include "../../../DataStream/PCLframe.h"

namespace kai
{
    class _PointCloud : public _GeometryBase
    {
    public:
        _PointCloud();
        virtual ~_PointCloud();

        bool link(InstanceMgr *pM) override;
        bool check(void) override;
        void clear(void) override;

    protected:
        PCLframe *m_pPCL = nullptr;
    };

}
#endif
