/*
 * PCbase.h
 *
 *  Created on: May 24, 2020
 *      Author: yankai
 */

#ifndef OpenKAI_src_Universe_Geometry__GeometryBase_H_
#define OpenKAI_src_Universe_Geometry__GeometryBase_H_

#include "../_ReferenceFrame.h"

namespace kai
{
    class _GeometryBase : public _ReferenceFrame
    {
    public:
        _GeometryBase();
        virtual ~_GeometryBase();

        virtual bool loadConfig(void) override;
        virtual bool saveConfig(bool bExport) override;
        virtual bool link(InstanceMgr *pM) override;
        virtual bool check(void);
        virtual void console(void *pConsole);

        virtual void clear(void);

    protected:

    };

}
#endif
