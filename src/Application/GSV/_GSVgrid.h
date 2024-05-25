#ifndef OpenKAI_src_Application_GSV__GSVgrid_H_
#define OpenKAI_src_Application_GSV__GSVgrid_H_

#include "../../3D/PointCloud/_PCgrid.h"
#include "_GSVio.h"

namespace kai
{
	struct GSV_GRID_CELL
	{
		vInt3 m_vCellIdx;
		uint32_t m_nPalarm;
	};

	class _GSVgrid : public _PCgrid
	{
	public:
		_GSVgrid();
		virtual ~_GSVgrid();

		virtual bool init(void *pKiss);
		virtual bool link(void);
        virtual bool start(void);
		virtual int check(void);

	protected:
		virtual void updateGSVgrid(void);

	private:
		void update(void);
		static void *getUpdate(void *This)
		{
			((_GSVgrid *)This)->update();
			return NULL;
		}

	protected:
		_GSVio* m_pGio;
		bool m_bAlarm;

		PC_GRID_ACTIVE_CELL *m_pCellAlert;
		PC_GRID_ACTIVE_CELL *m_pCellAlarm;
		PC_GRID_ACTIVE_CELL *m_pCellSelected;



	};

}
#endif
