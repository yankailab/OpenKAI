#ifndef OpenKAI_src_Control_clBase_H_
#define OpenKAI_src_Control_clBase_H_

#include "../../Base/BASE.h"
#include "../../Module/Kiss.h"

#define CL_TARGET_OPENCL_VERSION 300
#include <CL/cl.h>

#define CLBASE_SRC_NB 102400

namespace kai
{

	class clBase : public BASE
	{
	public:
		clBase();
		virtual ~clBase();

		virtual int init(void *pKiss);
		virtual int link(void);
		virtual int check(void);

	protected:
		virtual void getPlatformInfo(void);

		virtual bool setupCL(void);
		virtual bool setupKernel(void);

	protected:
		int m_iTargetPlatformIdx;
		int m_iTargetDevIdx;

		string m_targetPlatformName;
		string m_targetDevName;

		cl_device_id m_clDev;

		cl_context m_clContext;
		cl_context_properties* m_pCLcontextProps;
  		cl_queue_properties* m_pCLqProp;

		cl_command_queue m_clCmdQ;

		string m_fKernel;
		string m_buildOpt;
		string m_kName;
		cl_program m_clProgram;
		cl_kernel m_clKernel;
	};

}
#endif
