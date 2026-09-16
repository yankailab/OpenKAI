#ifndef OpenKAI_src_3D_Grid_OctreeGridID_H_
#define OpenKAI_src_3D_Grid_OctreeGridID_H_

#include "../../Primitive/UUID128.h"

namespace kai
{
	// 128-bit cell IDs: two reserved high bits, forty 3-bit child indices,
	// and a 6-bit depth (0 = root). Unused child indices are zero.
	constexpr int OCTGRID_MAX_LEVEL = 40;
}
#endif
