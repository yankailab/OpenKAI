#ifndef OpenKAI_src_Navigation__GeoFence_H_
#define OpenKAI_src_Navigation__GeoFence_H_

#include "../Protocol/_JSONbase.h"
#include "../Autopilot/FC/ArduPilot/_APmav_base.h"

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

		virtual bool loadConfig(void) override;
		virtual bool link(void) override;
		virtual bool start(void);
		virtual void console(void *pConsole);
		virtual void console(const json &j, void *pJSONbase);

		virtual bool saveConfig(void) override;

		virtual _GeoFence_TYPE getType(void);
		void setPosHdg(const Vector2d& vP, float hdgDeg);	// lat lon
		void setPolygon(const vector<vector<double> >& vvCoord);
		bool bBreach(void);
		void getP(Vector2d* pP, Vector2d* pPnext);

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
		Vector2d m_vP = Vector2d::Zero();		// vehicle current position, lat, lon order
		Vector2d m_vPnext = Vector2d::Zero();	// vehicle estimated next position
		vector<vector<double> > m_vPolygon;

		_JSONbase* m_pJb = nullptr;
	};

}
#endif
