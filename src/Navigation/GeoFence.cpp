#include "GeoFence.h"

namespace kai
{
	GeoFence::GeoFence()
	{
		m_type = geoFence_polygon;
	}

	GeoFence::~GeoFence()
	{
	}

	bool GeoFence::init(const json &j)
	{
		IF_F(!this->BASE::init(j));

		//		jKv(j, "next", m_next);

		return true;
	}

	bool GeoFence::link(const json &j, ModuleMgr *pM)
	{
		IF_F(!this->BASE::link(j, pM));

		return true;
	}

	GEOFENCE_TYPE GeoFence::getType(void)
	{
		return m_type;
	}

	void GeoFence::console(void *pConsole)
	{
		NULL_(pConsole);
		this->BASE::console(pConsole);

		// _Console *pC = (_Console *)pConsole;
		// if (!bActive())
		// 	pC->addMsg("[Inactive]", 0);
		// else
		// 	pC->addMsg("[ACTIVE]", 0);
	}

	void GeoFence::console(const json &j, void *pJSONbase)
	{
		_JSONbase *pJb = (_JSONbase *)pJSONbase;
		string cmd;
		IF_(!jKv(j, "cmd", cmd));

		if (cmd == "setSteerSpeed")
		{
			float steer = 0;
			jKv(j, "steer", steer);
			float speed = 0;
			jKv(j, "speed", speed);

			//			setSteerSpeed(steer, speed);

			NULL_(pJb);
			json jr = json::object();
			jr["cmd"] = "setSteerSpeed";
			jr["bSuccess"] = true;
			pJb->sendJson(jr);
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

	bool GeoFence::bBreach(const vector<vDouble2> &vCoord,
						   const vDouble2 &vPos,
						   float hdg,
						   float dist,
						   float *pRangle)
	{
		if (pRangle)
			*pRangle = hdg;

		IF_F(vCoord.size() < 3); // invalid polygon

		// Build local polygon around current position.
		vector<vDouble2> polyLocal;
		polyLocal.reserve(vCoord.size());
		for (const auto &g : vCoord)
			polyLocal.push_back(geoToLocalMeters(g, vPos));

		// Current robot position is origin in local frame.
		const vDouble2 p0{0.0, 0.0};

		// Heading convention: 0=N, 90=E
		const double hdgRad = static_cast<double>(hdg) * kDegToRad;
		const vDouble2 moveVec{
			static_cast<double>(dist) * std::sin(hdgRad), // East
			static_cast<double>(dist) * std::cos(hdgRad)  // North
		};

		const vDouble2 p1 = p0 + moveVec;

		// Compute the new geo position too
		// Not used directly after this line, but shown for completeness.
		const vDouble2 newGeo = localMetersToGeo(p1, vPos);
		(void)newGeo;

		// If new point is still inside (or on edge), no breach.
		IF_F(pointInPolygonOrOnEdge(polyLocal, p1));

		// New point is outside => breach.
		NULL__(pRangle, true);

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
				*pRangle = static_cast<float>(vectorToHeadingDeg(refl));
			}
		}

		return true;
	}

}
