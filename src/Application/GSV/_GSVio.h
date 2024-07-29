/*
 * _GSVdetect.h
 *
 *  Created on: Jan 14, 2024
 *      Author: yankai
 */

#ifndef OpenKAI_src_App_GSV__GSVio_H_
#define OpenKAI_src_App_GSV__GSVio_H_

#include "../../IO/_ADIObase.h"

namespace kai
{

    class _GSVio : public _ModuleBase
    {
    public:
        _GSVio();
        ~_GSVio();

        virtual int init(void *pKiss);
		virtual int link(void);
        virtual int start(void);
        virtual int check(void);
		virtual void console(void *pConsole);

        void setAlarm(bool bAlarm);

    protected:
        void setEnable(bool bEnable);

        virtual void updateAlarm(void);
        virtual void update(void);
        static void *getUpdate(void *This)
        {
            ((_GSVio *)This)->update();
            return NULL;
        }

    protected:
        _ADIObase* m_pDio;

        int m_iDinOn;
        int m_iDinOff;

        int m_iDoutLED;
        int m_iDoutNO;
        int m_iDoutNC;

        bool m_bAlarm;
        bool m_bEnable;

    };

}
#endif
