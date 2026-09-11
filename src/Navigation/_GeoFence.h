#ifndef OpenKAI_src_Navigation__GeoFence_H_
#define OpenKAI_src_Navigation__GeoFence_H_

#include "../Protocol/_JSONbase.h"
#include "../Autopilot/APmavlink/_APmavlink_base.h"

namespace kai
{
	enum _GeoFence_TYPE
	{
		_GeoFence_circle = 0,
		_GeoFence_polygon = 1,
	};

	class _GeoFence : public _ModuleBase
	{
	public:
		_GeoFence();
		~_GeoFence();

		virtual bool init(const json &j);
		virtual bool link(const json &j, ModuleMgr *pM);
		virtual bool start(void);
		virtual void console(void *pConsole);
		virtual void console(const json &j, void *pJSONbase);

		virtual bool loadConfig(json *pJ = nullptr, string fName = "");
		virtual bool saveConfig(json &j, string fName = "");

		virtual _GeoFence_TYPE getType(void);
		void setPosHdg(const vDouble2& vP, float hdgDeg);	// lat lon
		void setPolygon(const vector<vector<double> >& vvCoord);
		bool bBreach(void);
		void getP(vDouble2* pP, vDouble2* pPnext);

	private:
		virtual void sendFence(void);
		virtual void updateFencePolygon(void);
		virtual void update(void);
		static void *getUpdate(void *This)
		{
			((_GeoFence *)This)->update();
			return NULL;
		}

	protected:
		_GeoFence_TYPE m_type = _GeoFence_polygon;
		bool m_bBreach = false;

		float m_estD = 1;
		float m_hdg = 0;
		float m_rAngle = 0;		// reflected angle
		vDouble2 m_vP;		// vehicle current position, lat, lon order
		vDouble2 m_vPnext;	// vehicle estimated next position
		vector<vector<double> > m_vPolygon;

		_JSONbase* m_pJb = nullptr;
	};

}
#endif
