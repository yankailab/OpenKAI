#ifndef OpenKAI_src_Application_GSV__GSVgrid_H_
#define OpenKAI_src_Application_GSV__GSVgrid_H_

#include "../../3D/PointCloud/_PCgrid.h"

namespace kai
{

	class _GSVgrid : public _PCgrid
	{
	public:
		_GSVgrid();
		virtual ~_GSVgrid();

		virtual bool init(void *pKiss);
		virtual int check(void);


	protected:

	protected:


	};

}
#endif
