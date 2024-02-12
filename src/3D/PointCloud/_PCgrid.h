#ifndef OpenKAI_src_3D_PointCloud__PCgrid_H_
#define OpenKAI_src_3D_PointCloud__PCgrid_H_

#include "../_GeometryBase.h"

namespace kai
{
	struct PC_GRID_CELL
	{
		uint16_t m_nP; // nPoints inside the cell

		void clear(void)
		{
			m_nP = 0;
		}
	};

	class _PCgrid : public _GeometryBase
	{
	public:
		_PCgrid();
		virtual ~_PCgrid();

		virtual bool init(void *pKiss);
		virtual int check(void);

	protected:
		virtual bool generate(void);
        virtual void getStream(void* p);


	protected:
		PC_GRID_CELL *m_pCell;
		int m_nCell;
		vInt3 m_vDim;
		vFloat3 m_vCellSize;
		vInt3 m_vPorigin;

	};

}
#endif
