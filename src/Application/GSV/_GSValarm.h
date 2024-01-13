/*
 * _GSVdetect.h
 *
 *  Created on: Jan 14, 2024
 *      Author: yankai
 */

#ifndef OpenKAI_src_App_GSV__GSValarm_H_
#define OpenKAI_src_App_GSV__GSValarm_H_

#include "../../Universe/_Universe.h"
#include "../../IO/_ADIObase.h"
#include "../../Actuator/_Feetech.h"

namespace kai
{

    class _GSValarm : public _ModuleBase
    {
    public:
        _GSValarm();
        ~_GSValarm();

        virtual bool init(void *pKiss);
		virtual bool link(void);
        virtual bool start(void);
        virtual int check(void);
		virtual void console(void *pConsole);

    protected:
        virtual void updateGSV(void);
        virtual void update(void);
        static void *getUpdate(void *This)
        {
            ((_GSValarm *)This)->update();
            return NULL;
        }

    protected:
        vector<_Universe*> m_vpU;

        _ADIObase* m_pDio;
        _Feetech* m_pS;

        int m_nObj;

    };

}
#endif
