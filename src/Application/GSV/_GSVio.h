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

        virtual bool init(void *pKiss);
		virtual bool link(void);
        virtual bool start(void);
        virtual int check(void);
		virtual void console(void *pConsole);

        void setAlarm(bool bAlarm);

    protected:
        virtual void updateAlarm(void);
        virtual void update(void);
        static void *getUpdate(void *This)
        {
            ((_GSVio *)This)->update();
            return NULL;
        }

    protected:
        _ADIObase* m_pDio;
        int m_iDin;
        int m_iDout;
        int m_iDout2;

        bool m_bAlarm;
        bool m_bEnable;


    };

}
#endif
