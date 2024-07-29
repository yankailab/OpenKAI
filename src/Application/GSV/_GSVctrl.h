/*
 * _GSVctrl.h
 *
 *  Created on: May 28, 2020
 *      Author: yankai
 */

#ifndef OpenKAI_src_Application_GSV__GSVctrl_H_
#define OpenKAI_src_Application_GSV__GSVctrl_H_

#include "../../Protocol/_JSONbase.h"
#include "../../3D/_GeometryBase.h"
#include "_GSVgrid.h"

namespace kai
{

	class _GSVctrl : public _JSONbase
	{
	public:
		_GSVctrl();
		virtual ~_GSVctrl();

		virtual int init(void *pKiss);
		virtual int link(void);
		virtual int start(void);
		virtual int check(void);
		virtual void console(void *pConsole);

	private:
		// main thread
		void send(void);
		void sendHeartbeat(void);
		void sendConfig(void);
		void update(void);
		static void *getUpdate(void *This)
		{
			((_GSVctrl *)This)->update();
			return NULL;
		}

		// UI handler
		void updateGrid(picojson::object &o);
		void setGrid(picojson::object &o);
		void saveGrid(picojson::object &o);

		void updateTR(picojson::object &o);
		void setTR(picojson::object &o);
		void saveGeometryConfig(picojson::object &o);

		void setTRall(picojson::object &o);
		void saveGeometryConfigAll(picojson::object &o);

		void selectCell(picojson::object &o);
		void setCellAlert(picojson::object &o);
		void delCellAlert(picojson::object &o);
		void saveGridConfig(picojson::object &o);

		void setParams(picojson::object &o);

		void autoAlertCells(picojson::object &o);

		void handleMsg(const string &str);
		void updateR(void);
		static void *getUpdateR(void *This)
		{
			((_GSVctrl *)This)->updateR();
			return NULL;
		}

		// geometry control
		_GeometryBase* getGeometry(const string& n);

	protected:
		_GSVgrid* m_pGgrid;
		vector<_GeometryBase*> m_vpGB;
		string m_msg;
	};

}
#endif
