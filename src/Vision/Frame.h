/*
 * Frame.h
 *
 *  Created on: Aug 21, 2015
 *      Author: yankai
 */

#ifndef OpenKAI_src_Vision_Frame_H_
#define OpenKAI_src_Vision_Frame_H_

#include "FrameBase.h"
#include "FrameGPU.h"

namespace kai
{
#ifdef USE_CUDA
    typedef FrameGPU Frame;
#else
    typedef FrameBase Frame;
#endif
}
#endif
