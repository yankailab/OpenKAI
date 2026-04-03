#include "_GeoFence.h"

namespace kai
{
	_GeoFence::_GeoFence()
	{
		m_pAP = nullptr;

		m_type = _GeoFence_polygon;
		m_bBreach = false;
		m_estD = 1;
		m_hdg = 0;
		m_rAngle = 0;
		m_vP.set(0);
		m_vPnext.set(0);
		m_pJb = nullptr;
	}

	_GeoFence::~_GeoFence()
	{
	}

	bool _GeoFence::init(const json &j)
	{
		IF_F(!this->_ModuleBase::init(j));

		jKv(j, "type", m_type);
		jKv(j, "estD", m_estD);
		jKv<double>(j, "vP", m_vP);
		jKv(j, "vPolygon", m_vPolygon);

		return true;
	}

	bool _GeoFence::link(const json &j, ModuleMgr *pM)
	{
		IF_F(!this->_ModuleBase::link(j, pM));

		string n = "";
		jKv(j, "_JSONbase", n);
		m_pJb = (_JSONbase *)(pM->findModule(n));

		n = "";
		jKv(j, "_APmavlink_base", n);
		m_pAP = (_APmavlink_base *)(pM->findModule(n));

		return true;
	}

	bool _GeoFence::start(void)
	{
		NULL_F(m_pT);
		return m_pT->startThread(getUpdate, this);
	}

	void _GeoFence::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPS();

			if(m_pAP)
			{
				vDouble4 vPos = m_pAP->getGlobalPos();
				float hdg = m_pAP->getHdg();

				setPosHdg(vDouble2(vPos.x, vPos.y), hdg);
			}

			updateFencePolygon();
			sendFence();
		}
	}

	_GeoFence_TYPE _GeoFence::getType(void)
	{
		return m_type;
	}

	void _GeoFence::sendFence(void)
	{
		NULL_(m_pJb);

		json j = json::object();
		j["cmd"] = "geoFence";
		j["vP"] = {m_vP.x, m_vP.y}; // lat lon
		j["vPnext"] = {m_vPnext.x, m_vPnext.y};
		j["bBreach"] = m_bBreach;
		m_pJb->sendJson(j);
	}

	void _GeoFence::setPosHdg(const vDouble2 &vP, float hdgDeg)
	{
		m_vP = vP;
		m_hdg = hdgDeg;
	}

	void _GeoFence::setPolygon(const vector<vector<double>> &vvCoord)
	{
		m_vPolygon = vvCoord;
	}

	bool _GeoFence::bBreach(void)
	{
		return m_bBreach;
	}

	void _GeoFence::getP(vDouble2 *pP, vDouble2 *pPnext)
	{
		if (pP)
		{
			*pP = m_vP;
		}

		if (pPnext)
		{
			*pPnext = m_vPnext;
		}
	}

	void _GeoFence::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_ModuleBase::console(pConsole);

		_Console *pC = (_Console *)pConsole;
		pC->addMsg("bBreach: " + i2str(m_bBreach));
		pC->addMsg("nPolygonVertices:" + i2str(m_vPolygon.size()));
		pC->addMsg("vP: (" + lf2str(m_vP.x, 7) + ", " + lf2str(m_vP.y, 7) + ")");
		pC->addMsg("hdg: " + f2str(m_hdg));
	}

	void _GeoFence::console(const json &j, void *pJSONbase)
	{
		_JSONbase *pJb = (_JSONbase *)pJSONbase;
		string cmd;
		IF_(!jKv(j, "cmd", cmd));

		if (cmd == "setGeoFence")
		{
			string type;
			jKv(j, "type", type);

			if (type == "polygon")
			{
				jKv(j, "vPolygon", m_vPolygon);
				setPolygon(m_vPolygon);
			}
			else if (type == "circle")
			{
			}

			NULL_(pJb);
			json jr = json::object();
			jr["cmd"] = "set_GeoFence";
			jr["bSuccess"] = true;
			pJb->sendJson(jr);

			m_pJb = pJb;
		}
	}

	constexpr double kPi = 3.14159265358979323846;
	constexpr double kDegToRad = kPi / 180.0;
	constexpr double kRadToDeg = 180.0 / kPi;
	constexpr double kEarthRadiusM = 6378137.0;
	constexpr double kEps = 1e-9;

	inline double normalizeHeadingDeg(double deg)
	{
		while (deg < 0.0)
			deg += 360.0;
		while (deg >= 360.0)
			deg -= 360.0;
		return deg;
	}

	inline double dot2(const vDouble2 &a, const vDouble2 &b)
	{
		return a.x * b.x + a.y * b.y;
	}

	inline double cross2(const vDouble2 &a, const vDouble2 &b)
	{
		return a.x * b.y - a.y * b.x;
	}

	inline vDouble2 operator+(const vDouble2 &a, const vDouble2 &b)
	{
		return {a.x + b.x, a.y + b.y};
	}

	inline vDouble2 operator-(const vDouble2 &a, const vDouble2 &b)
	{
		return {a.x - b.x, a.y - b.y};
	}

	inline vDouble2 operator*(const vDouble2 &a, double s)
	{
		return {a.x * s, a.y * s};
	}

	inline double norm2(const vDouble2 &a)
	{
		return std::sqrt(dot2(a, a));
	}

	inline vDouble2 normalize2(const vDouble2 &a)
	{
		const double n = norm2(a);
		if (n < kEps)
			return {0.0, 0.0};
		return {a.x / n, a.y / n};
	}

	// Convert geo point (lat, lon) to local ENU-like planar coordinates in meters:
	// local.x = East, local.y = North
	// Reference origin = refGeo
	inline vDouble2 geoToLocalMeters(const vDouble2 &geo, const vDouble2 &refGeo)
	{
		const double refLatRad = refGeo.x * kDegToRad;
		const double dLatRad = (geo.x - refGeo.x) * kDegToRad;
		const double dLonRad = (geo.y - refGeo.y) * kDegToRad;

		const double north = kEarthRadiusM * dLatRad;
		const double east = kEarthRadiusM * std::cos(refLatRad) * dLonRad;

		return {east, north};
	}

	inline vDouble2 localMetersToGeo(const vDouble2 &local, const vDouble2 &refGeo)
	{
		const double refLatRad = refGeo.x * kDegToRad;

		const double dLatRad = local.y / kEarthRadiusM;
		const double dLonRad = local.x / (kEarthRadiusM * std::cos(refLatRad));

		const double lat = refGeo.x + dLatRad * kRadToDeg;
		const double lon = refGeo.y + dLonRad * kRadToDeg;

		return {lat, lon};
	}

	inline bool pointOnSegment(const vDouble2 &p, const vDouble2 &a, const vDouble2 &b)
	{
		const vDouble2 ab = b - a;
		const vDouble2 ap = p - a;
		const double area2 = std::fabs(cross2(ab, ap));
		if (area2 > 1e-7)
			return false;

		const double d = dot2(ap, ab);
		if (d < -1e-7)
			return false;

		const double ab2 = dot2(ab, ab);
		if (d > ab2 + 1e-7)
			return false;

		return true;
	}

	// Ray casting. Boundary counts as inside.
	bool pointInPolygonOrOnEdge(const vector<vDouble2> &poly, const vDouble2 &p)
	{
		const size_t n = poly.size();
		if (n < 3)
			return false;

		bool inside = false;
		for (size_t i = 0, j = n - 1; i < n; j = i++)
		{
			const vDouble2 &a = poly[j];
			const vDouble2 &b = poly[i];

			if (pointOnSegment(p, a, b))
				return true;

			const bool intersect =
				((a.y > p.y) != (b.y > p.y)) &&
				(p.x < (b.x - a.x) * (p.y - a.y) / ((b.y - a.y) + 1e-30) + a.x);

			if (intersect)
				inside = !inside;
		}
		return inside;
	}

	// Segment-segment intersection:
	// p + t*r intersects a + u*s
	// Returns true if intersecting, and outputs t/u/intersection.
	bool segmentIntersect(
		const vDouble2 &p0,
		const vDouble2 &p1,
		const vDouble2 &a,
		const vDouble2 &b,
		double &outT,
		double &outU,
		vDouble2 &outPt)
	{
		const vDouble2 r = p1 - p0;
		const vDouble2 s = b - a;
		const double denom = cross2(r, s);
		const vDouble2 ap = a - p0;

		if (std::fabs(denom) < kEps)
		{
			// Parallel or collinear: ignore for reflection purposes here.
			return false;
		}

		const double t = cross2(ap, s) / denom;
		const double u = cross2(ap, r) / denom;

		if (t >= -1e-9 && t <= 1.0 + 1e-9 &&
			u >= -1e-9 && u <= 1.0 + 1e-9)
		{
			outT = t;
			outU = u;
			outPt = p0 + r * t;
			return true;
		}

		return false;
	}

	// Reflect direction d across the infinite line whose tangent is edgeUnit.
	inline vDouble2 reflectAcrossEdgeTangent(const vDouble2 &d, const vDouble2 &edgeUnit)
	{
		// Reflection across line: r = 2*proj_line(d) - d
		const double proj = dot2(d, edgeUnit);
		return edgeUnit * (2.0 * proj) - d;
	}

	// Convert local motion vector to navigation heading:
	// x = East, y = North, heading 0=N, 90=E
	inline double vectorToHeadingDeg(const vDouble2 &v)
	{
		const double hdgRad = std::atan2(v.x, v.y); // atan2(East, North)
		return normalizeHeadingDeg(hdgRad * kRadToDeg);
	}

	void _GeoFence::updateFencePolygon(void)
	{
		IF_(m_vPolygon.size() < 3); // invalid polygon

		// Build local polygon around current position.
		vector<vDouble2> polyLocal;
		polyLocal.reserve(m_vPolygon.size());
		for (const auto &g : m_vPolygon)
			polyLocal.push_back(geoToLocalMeters(vDouble2(g[0], g[1]), m_vP));

		// Current robot position is origin in local frame.
		const vDouble2 p0{0.0, 0.0};

		// Heading convention: 0=N, 90=E
		const double hdgRad = static_cast<double>(m_hdg) * kDegToRad;
		const vDouble2 moveVec{
			static_cast<double>(m_estD) * std::sin(hdgRad), // East
			static_cast<double>(m_estD) * std::cos(hdgRad)	// North
		};

		const vDouble2 p1 = p0 + moveVec;

		// Compute the new geo position
		m_vPnext = localMetersToGeo(p1, m_vP);

		// If new point is still inside (or on edge), no breach.
		if (pointInPolygonOrOnEdge(polyLocal, p1))
		{
			m_bBreach = false;
			m_rAngle = 0;
			return;
		}

		// Breach
		m_bBreach = true;

		// Compute reflection heading from the crossed edge.
		double bestT = std::numeric_limits<double>::infinity();
		bool foundEdge = false;
		vDouble2 bestA{}, bestB{}, ip{};

		const size_t n = polyLocal.size();
		for (size_t i = 0; i < n; ++i)
		{
			const vDouble2 &a = polyLocal[i];
			const vDouble2 &b = polyLocal[(i + 1) % n];

			double t = 0.0, u = 0.0;
			vDouble2 hit{};
			if (segmentIntersect(p0, p1, a, b, t, u, hit))
			{
				// Ignore the trivial start point touch if already exactly on boundary.
				if (t < 1e-8)
					continue;

				if (t < bestT)
				{
					bestT = t;
					bestA = a;
					bestB = b;
					ip = hit;
					foundEdge = true;
				}
			}
		}

		if (foundEdge)
		{
			const vDouble2 edge = bestB - bestA;
			const vDouble2 edgeUnit = normalize2(edge);
			const vDouble2 dirUnit = normalize2(moveVec);

			if (norm2(edgeUnit) > kEps && norm2(dirUnit) > kEps)
			{
				const vDouble2 refl = reflectAcrossEdgeTangent(dirUnit, edgeUnit);
				m_rAngle = static_cast<float>(vectorToHeadingDeg(refl));
			}
		}
	}

}
