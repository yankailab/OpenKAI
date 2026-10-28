#ifndef OpenKAI_src_Navigation__GeoFence_H_
#define OpenKAI_src_Navigation__GeoFence_H_

#include "../Protocol/_JSONbase.h"
#include <mutex>

namespace kai
{
	enum _GeoFence_TYPE
	{
		_GeoFence_circle = 0,
		_GeoFence_polygon = 1,
	};

	struct _GeoFence_STATUS
	{
		bool bValid = false;
		bool bCurrentBreach = false;
		bool bPredictedBreach = false; // endpoint outside, or forward segment exits the polygon
		bool bTurnRequired = false; // false also permits an outside rover progressing toward re-entry
		float attackDeg = 0; // signed incidence from the outward normal; positive clockwise
		float reflectedHdgDeg = 0; // navigation heading, 0 = north, 90 = east
		float turnDeg = 0; // shortest turn to reflectedHdgDeg, positive right, negative left
	};

	class _GeoFence : public _ModuleBase
	{
	public:
		_GeoFence();
		~_GeoFence();

		virtual bool loadConfig(void) override;
		virtual bool link(InstanceMgr *pM) override;
		virtual bool start(void);
		virtual void console(void *pConsole);
		virtual void console(const json &j, void *pJSONbase);

		virtual bool saveConfig(bool bExport) override;

		virtual _GeoFence_TYPE getType(void);
		// Evaluates immediately; no fence worker tick is required before reading status.
		void setPosHdg(const Vector2d& vP, float hdgDeg);	// lat lon
		void setPolygon(const vector<vector<double> >& vvCoord);
		bool bBreach(void); // current OR predicted breach; use getStatus().bTurnRequired for recovery
		_GeoFence_STATUS getStatus(void); // coherent snapshot; check bValid before navigating
		void getP(Vector2d* pP, Vector2d* pPnext);

	private:
		virtual void sendFence(void);
		virtual void updateFencePolygon(void);
		void evaluateFencePolygon(void); // caller holds m_fenceMutex
		virtual void update(void);
		static void *getUpdate(void *This)
		{
			((_GeoFence *)This)->update();
			return NULL;
		}

	protected:
		_GeoFence_TYPE m_type = _GeoFence_polygon;
		std::mutex m_fenceMutex;
		_GeoFence_STATUS m_status;

		float m_estD = 1;
		float m_hdg = 0;
		Vector2d m_vP = Vector2d::Zero();		// vehicle current position, lat, lon order
		Vector2d m_vPnext = Vector2d::Zero();	// vehicle estimated next position
		vector<vector<double> > m_vPolygon;

		_JSONbase* m_pJb = nullptr;
	};

}
#endif
