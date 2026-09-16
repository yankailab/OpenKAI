#ifndef OpenKAI_src_Base_common_H_
#define OpenKAI_src_Base_common_H_

#ifdef USE_OPEN3D
#include "open3d.h"
#endif

#ifdef USE_OPENCV
#include "cv.h"
#endif

#ifdef USE_MATHGL
#include <mgl2/mgl.h>
#include <mgl2/fltk.h>
#endif

#ifdef USE_GLOG
#include <glog/logging.h>
#endif

#include <Eigen/Dense>
using namespace Eigen;

#include "platform.h"
#include "macro.h"
#include "constant.h"
#include "../Primitive/tSwap.h"
#include "../Utility/util.h"
#include "../Utility/utilTime.h"
#include "../Utility/utilEvent.h"
#include "../Utility/utilStr.h"

#endif

