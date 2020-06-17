#ifndef OpenKAI_src_Base_common_H_
#define OpenKAI_src_Base_common_H_

#include "../Config/switch.h"

#ifdef USE_OPEN3D
#include "open3d.h"
#endif

#ifdef USE_OPENCV
#include "cv.h"
#endif

#ifdef USE_JETSON_INFERENCE
#include "jetsonInf.h"
#endif

#include "platform.h"
#include "macro.h"
#include "constant.h"
#include <glog/logging.h>
#include "../Primitive/helper.h"
#include "../Primitive/vInt2.h"
#include "../Primitive/vInt3.h"
#include "../Primitive/vInt4.h"
#include "../Primitive/vFloat2.h"
#include "../Primitive/vFloat3.h"
#include "../Primitive/vFloat4.h"
#include "../Primitive/vDouble2.h"
#include "../Primitive/vDouble3.h"
#include "../Primitive/vDouble4.h"
#include "../Utility/util.h"

#endif

