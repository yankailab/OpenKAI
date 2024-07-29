#ifndef OpenKAI_src_Application_GSV__GSVgrid_H_
#define OpenKAI_src_Application_GSV__GSVgrid_H_

#include "../../3D/PointCloud/_PCgrid.h"
#include "../../IO/_File.h"
#include "../../Utility/utilFile.h"
#include "_GSVio.h"

namespace kai
{
	class _GSVgrid : public _PCgrid
	{
	public:
		_GSVgrid();
		virtual ~_GSVgrid();

		virtual int init(void *pKiss);
		virtual int link(void);
        virtual int start(void);
		virtual int check(void);

		bool selectCell(const vInt3 vC);
		bool addAlertCell(const vInt3 vC);
		void deleteAlertCell(const vInt3 vC);
		void calibAlertCellNpAlarm(void);

		void setNpAlertSensitivity(int s);
		void autoFindAlertCells(void);

		bool loadConfig(void);
		bool saveConfig(void);

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
		string m_fConfig;
		_GSVio* m_pGio;
		bool m_bAlarm;

		int m_nPalertSensitivity;
		vInt3 m_vCselected;

		PC_GRID_ACTIVE_CELL *m_pCellAlert;
		PC_GRID_ACTIVE_CELL *m_pCellAlarm;
		PC_GRID_ACTIVE_CELL *m_pCellSelected;

	};

}
#endif
