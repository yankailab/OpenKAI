/*
 * _PCdownSample.h
 *
 *  Created on: Feb 7, 2021
 *      Author: yankai
 */

#ifndef OpenKAI_src_3D_PointCloud_PCdownSample_H_
#define OpenKAI_src_3D_PointCloud_PCdownSample_H_

#include "../../_GeometryBase.h"

namespace kai
{

    class _PCdownSample : public _GeometryBase
    {
    public:
        _PCdownSample();
        virtual ~_PCdownSample();

        virtual bool init(const json &j);
        virtual bool start(void);
        virtual bool check(void);

    private:
        virtual void update(void);
        void updateFilter(void);
        static void *getUpdate(void *This)
        {
            ((_PCdownSample *)This)->update();
            return NULL;
        }

    protected:
        float m_rVoxel;
    };

}
#endif
