#ifndef OpenKAI_src_Application_GSV__GSVcollisionAvoid_H_
#define OpenKAI_src_Application_GSV__GSVcollisionAvoid_H_

#include "../../Vision/RGBD/_RGBDbase.h"
#include "../../UI/_Console.h"

namespace kai
{

    class _GSVcollisionAvoid : public _ModuleBase
    {
    public:
        _GSVcollisionAvoid();
        ~_GSVcollisionAvoid();

        virtual bool init(void *pKiss);
		virtual bool link(void);
        virtual bool start(void);
        virtual int check(void);
        virtual void update(void);
		virtual void console(void *pConsole);
		virtual void draw(void *pFrame);

    protected:
        virtual void updateGSV(void);
        static void *getUpdate(void *This)
        {
            ((_GSVcollisionAvoid *)This)->update();
            return NULL;
        }

    public:

    };

}
#endif
