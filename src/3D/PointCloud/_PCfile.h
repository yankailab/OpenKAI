/*
 * _PCfile.h
 *
 *  Created on: Sept 3, 2020
 *      Author: yankai
 */

#ifndef OpenKAI_src_3D_PointCloud_PCfile_H_
#define OpenKAI_src_3D_PointCloud_PCfile_H_

#include "_PCframe.h"

namespace kai
{

	class _PCfile : public _PCframe
	{
	public:
		_PCfile();
		virtual ~_PCfile();

		int init(void *pKiss);
		int start(void);
		bool open(void);

	private:
		void update(void);
		static void *getUpdate(void *This)
		{
			((_PCfile *)This)->update();
			return NULL;
		}

	public:
		vector<string> m_vfName;
		PointCloud m_pcl;
	};

}
#endif
