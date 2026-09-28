/*
 * _Line.h
 *
 *  Created on: May 24, 2020
 *      Author: yankai
 */

#ifndef OpenKAI_src_Universe_Geometry_Line__Line_H_
#define OpenKAI_src_Universe_Geometry_Line__Line_H_

#include "../_GeometryBase.h"
#include "../../../DataStream/LineFrame.h"

namespace kai
{
    class _Line : public _GeometryBase
    {
    public:
        _Line();
        virtual ~_Line();

        bool link(InstanceMgr *pM) override;
        bool check(void) override;
        void clear(void) override;

    protected:
        LineFrame *m_pLine = nullptr;
    };

}
#endif
