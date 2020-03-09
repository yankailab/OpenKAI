#ifndef OpenKAI_src_Base_common_H_
#define OpenKAI_src_Base_common_H_

#include "../Config/switch.h"

#ifdef USE_OCR
#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>
#endif

#include "cv.h"
#include "tensorRT.h"
#include "platform.h"
#include "macro.h"
#include "constant.h"
#include <glog/logging.h>
#include "../Primitive/data.h"
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

