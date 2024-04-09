#ifndef OpenKAI_src_Control_clBase_H_
#define OpenKAI_src_Control_clBase_H_

#include "../../Base/BASE.h"
#include "../../Script/Kiss.h"

#define CL_TARGET_OPENCL_VERSION 300
#include <CL/cl.h>

namespace kai
{

	class clBase : public BASE
	{
	public:
		clBase();
		virtual ~clBase();

		virtual bool init(void *pKiss);

	protected:
		virtual void update(void);

	public:
		uint64_t m_tLastUpdate;
	};

}
#endif
