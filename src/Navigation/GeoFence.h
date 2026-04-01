#ifndef OpenKAI_src_Navigation_GeoFence_H_
#define OpenKAI_src_Navigation_GeoFence_H_

#include "../Protocol/_JSONbase.h"

namespace kai
{
	enum GEOFENCE_TYPE
	{
		geoFence_circle = 0,
		geoFence_polygon = 1,
	};

	class GeoFence : public BASE
	{
	public:
		GeoFence();
		~GeoFence();

		virtual bool init(const json &j);
		virtual bool link(const json &j, ModuleMgr *pM);
		virtual void console(void *pConsole);
		virtual void console(const json &j, void *pJSONbase);

		virtual GEOFENCE_TYPE getType(void);
		bool bBreach(const vector<vDouble2> &vCoord,
					 const vDouble2 &vPos,
					 float hdg,
					 float dist,
					 float *pRangle = nullptr);

	protected:
		GEOFENCE_TYPE m_type;

		vector<vDouble2> m_vPolygon;
	};

}
#endif
