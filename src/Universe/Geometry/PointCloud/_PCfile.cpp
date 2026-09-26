/*
 * _PCfile.cpp
 *
 *  Created on: Sept 3, 2020
 *      Author: yankai
 */

#include "_PCfile.h"

#include <cerrno>
#include <cctype>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <limits>
#include <unistd.h>

namespace
{
	struct PLY_OUTPUT_FILE
	{
		string path;
		int descriptor = -1;
		FILE *stream = nullptr;
		bool created = false;
		~PLY_OUTPUT_FILE()
		{
			if (stream) fclose(stream);
			else if (descriptor >= 0) close(descriptor);
			if (created) std::remove(path.c_str());
		}
	};

	enum PLY_FORMAT
	{
		ply_format_unknown = 0,
		ply_format_ascii,
		ply_format_binary_little_endian,
		ply_format_binary_big_endian,
	};

	enum PLY_TYPE
	{
		ply_type_invalid = 0,
		ply_type_int8,
		ply_type_uint8,
		ply_type_int16,
		ply_type_uint16,
		ply_type_int32,
		ply_type_uint32,
		ply_type_float32,
		ply_type_float64,
	};

	struct PLY_PROPERTY
	{
		bool m_bList = false;
		string m_name;
		PLY_TYPE m_type = ply_type_invalid;
		PLY_TYPE m_typeCount = ply_type_invalid;
		PLY_TYPE m_typeItem = ply_type_invalid;
	};

	struct PLY_ELEMENT
	{
		string m_name;
		uint64_t m_n = 0;
		vector<PLY_PROPERTY> m_vProp;
	};

	struct PLY_VALUE
	{
		double m_v = 0.0;
		uint64_t m_raw = 0;
		PLY_TYPE m_type = ply_type_invalid;
	};

	string toLower(const string &s)
	{
		string r = s;
		for (char &c : r)
			c = (char)tolower((unsigned char)c);

		return r;
	}

	void trimLineEnd(string *pS)
	{
		while (!pS->empty() && (pS->back() == '\r' || pS->back() == '\n'))
			pS->pop_back();
	}

	string u642str(uint64_t v)
	{
		stringstream ss;
		ss << v;
		return ss.str();
	}

	PLY_TYPE parseType(const string &s)
	{
		string t = toLower(s);

		if (t == "char" || t == "int8")
			return ply_type_int8;
		if (t == "uchar" || t == "uint8" || t == "unsigned_char")
			return ply_type_uint8;
		if (t == "short" || t == "int16")
			return ply_type_int16;
		if (t == "ushort" || t == "uint16" || t == "unsigned_short")
			return ply_type_uint16;
		if (t == "int" || t == "int32")
			return ply_type_int32;
		if (t == "uint" || t == "uint32" || t == "unsigned_int")
			return ply_type_uint32;
		if (t == "float" || t == "float32")
			return ply_type_float32;
		if (t == "double" || t == "float64")
			return ply_type_float64;

		return ply_type_invalid;
	}

	int typeSize(PLY_TYPE t)
	{
		switch (t)
		{
		case ply_type_int8:
		case ply_type_uint8:
			return 1;
		case ply_type_int16:
		case ply_type_uint16:
			return 2;
		case ply_type_int32:
		case ply_type_uint32:
		case ply_type_float32:
			return 4;
		case ply_type_float64:
			return 8;
		default:
			return 0;
		}
	}

	bool bIntegerType(PLY_TYPE t)
	{
		return t == ply_type_int8 ||
			   t == ply_type_uint8 ||
			   t == ply_type_int16 ||
			   t == ply_type_uint16 ||
			   t == ply_type_int32 ||
			   t == ply_type_uint32;
	}

	bool bSignedType(PLY_TYPE t)
	{
		return t == ply_type_int8 ||
			   t == ply_type_int16 ||
			   t == ply_type_int32;
	}

	bool parseDouble(const string &s, double *pV)
	{
		NULL_F(pV);

		char *pEnd = nullptr;
		errno = 0;
		double v = strtod(s.c_str(), &pEnd);
		IF_F(pEnd == s.c_str());
		IF_F(*pEnd != 0);
		IF_F(errno == ERANGE);

		*pV = v;
		return true;
	}

	uint64_t bytesToUint(const uint8_t *pB, int nB, bool bLittleEndian)
	{
		uint64_t v = 0;

		if (bLittleEndian)
		{
			for (int i = nB - 1; i >= 0; i--)
				v = (v << 8) | pB[i];
		}
		else
		{
			for (int i = 0; i < nB; i++)
				v = (v << 8) | pB[i];
		}

		return v;
	}

	int64_t signExtend(uint64_t v, int nBit)
	{
		uint64_t bitSign = 1ULL << (nBit - 1);
		if (v & bitSign)
			return (int64_t)(v | (~0ULL << nBit));

		return (int64_t)v;
	}

	bool readAsciiValue(istream &is, PLY_TYPE t, PLY_VALUE *pV, string *pErr)
	{
		NULL_F(pV);
		string token;
		if (!(is >> token))
		{
			if (pErr)
				*pErr = "Unexpected end of ASCII PLY data";
			return false;
		}

		double v = 0.0;
		if (!parseDouble(token, &v))
		{
			if (pErr)
				*pErr = "Invalid numeric value in ASCII PLY data: " + token;
			return false;
		}

		pV->m_v = v;
		pV->m_type = t;
		if (bSignedType(t))
			pV->m_raw = (uint64_t)(int64_t)v;
		else
			pV->m_raw = (uint64_t)v;

		return true;
	}

	bool readBinaryValue(istream &is, PLY_TYPE t, bool bLittleEndian, PLY_VALUE *pV, string *pErr)
	{
		NULL_F(pV);

		int nB = typeSize(t);
		if (nB <= 0)
		{
			if (pErr)
				*pErr = "Unsupported binary PLY property type";
			return false;
		}

		uint8_t b[8] = {0};
		if (!is.read((char *)b, nB))
		{
			if (pErr)
				*pErr = "Unexpected end of binary PLY data";
			return false;
		}

		uint64_t raw = bytesToUint(b, nB, bLittleEndian);
		pV->m_raw = raw;
		pV->m_type = t;

		switch (t)
		{
		case ply_type_int8:
		case ply_type_int16:
		case ply_type_int32:
			pV->m_v = (double)signExtend(raw, nB * 8);
			break;

		case ply_type_uint8:
		case ply_type_uint16:
		case ply_type_uint32:
			pV->m_v = (double)raw;
			break;

		case ply_type_float32:
		{
			uint32_t rv = (uint32_t)raw;
			float fv = 0.0f;
			memcpy(&fv, &rv, sizeof(float));
			pV->m_v = (double)fv;
			break;
		}

		case ply_type_float64:
		{
			double dv = 0.0;
			memcpy(&dv, &raw, sizeof(double));
			pV->m_v = dv;
			break;
		}

		default:
			if (pErr)
				*pErr = "Unsupported binary PLY property type";
			return false;
		}

		return true;
	}

	bool readValue(istream &is, PLY_TYPE t, PLY_FORMAT format, PLY_VALUE *pV, string *pErr)
	{
		if (format == ply_format_ascii)
			return readAsciiValue(is, t, pV, pErr);

		return readBinaryValue(is, t, format == ply_format_binary_little_endian, pV, pErr);
	}

	bool skipAsciiListItems(istream &is, uint64_t nItem, string *pErr)
	{
		string token;
		for (uint64_t i = 0; i < nItem; i++)
		{
			if (!(is >> token))
			{
				if (pErr)
					*pErr = "Unexpected end of ASCII PLY list data";
				return false;
			}
		}

		return true;
	}

	bool skipBinaryScalars(istream &is, PLY_TYPE t, uint64_t nItem, string *pErr)
	{
		int nB = typeSize(t);
		if (nB <= 0)
		{
			if (pErr)
				*pErr = "Unsupported binary PLY list item type";
			return false;
		}

		uint64_t nSkip = nItem * (uint64_t)nB;
		if (nItem > 0 && nSkip / nItem != (uint64_t)nB)
		{
			if (pErr)
				*pErr = "Binary PLY list is too large to skip";
			return false;
		}

		is.seekg((streamoff)nSkip, ios::cur);
		if (!is.good())
		{
			if (pErr)
				*pErr = "Unexpected end of binary PLY list data";
			return false;
		}

		return true;
	}

	bool skipProperty(istream &is, const PLY_PROPERTY &p, PLY_FORMAT format, string *pErr)
	{
		PLY_VALUE v;
		if (!p.m_bList)
			return readValue(is, p.m_type, format, &v, pErr);

		IF_F(!readValue(is, p.m_typeCount, format, &v, pErr));

		if (v.m_v < 0.0)
		{
			if (pErr)
				*pErr = "Negative list count in PLY data";
			return false;
		}

		uint64_t nItem = (uint64_t)v.m_v;
		if (format == ply_format_ascii)
			return skipAsciiListItems(is, nItem, pErr);

		return skipBinaryScalars(is, p.m_typeItem, nItem, pErr);
	}

	float clamp01(double v)
	{
		if (v < 0.0)
			return 0.0f;
		if (v > 1.0)
			return 1.0f;

		return (float)v;
	}

	double colorMax(PLY_TYPE t)
	{
		switch (t)
		{
		case ply_type_int8:
			return 127.0;
		case ply_type_uint8:
			return 255.0;
		case ply_type_int16:
			return 32767.0;
		case ply_type_uint16:
			return 65535.0;
		case ply_type_int32:
			return 2147483647.0;
		case ply_type_uint32:
			return 4294967295.0;
		default:
			return 1.0;
		}
	}

	float normalizeColor(const PLY_VALUE &v)
	{
		double c = v.m_v;
		if (bIntegerType(v.m_type))
			c /= colorMax(v.m_type);
		else if (c > 1.0)
			c /= 255.0;

		return clamp01(c);
	}

	void unpackRGB(uint32_t rgb, Vector3f *pC)
	{
		NULL_(pC);

		pC->x() = (float)((rgb >> 16) & 0xFF) / 255.0f;
		pC->y() = (float)((rgb >> 8) & 0xFF) / 255.0f;
		pC->z() = (float)(rgb & 0xFF) / 255.0f;
	}

	bool setVertexScalar(const PLY_PROPERTY &p, const PLY_VALUE &v, kai::GEOMETRY_POINT *pPt)
	{
		NULL_F(pPt);

		string n = toLower(p.m_name);

		if (n == "x")
		{
			pPt->m_vP.x() = (float)v.m_v;
			return true;
		}
		if (n == "y")
		{
			pPt->m_vP.y() = (float)v.m_v;
			return true;
		}
		if (n == "z")
		{
			pPt->m_vP.z() = (float)v.m_v;
			return true;
		}

		if (n == "red" || n == "r" || n == "diffuse_red")
		{
			pPt->m_vC.x() = normalizeColor(v);
			return true;
		}
		if (n == "green" || n == "g" || n == "diffuse_green")
		{
			pPt->m_vC.y() = normalizeColor(v);
			return true;
		}
		if (n == "blue" || n == "b" || n == "diffuse_blue")
		{
			pPt->m_vC.z() = normalizeColor(v);
			return true;
		}
		if (n == "rgb" || n == "rgba")
		{
			unpackRGB((uint32_t)v.m_raw, &pPt->m_vC);
			return true;
		}

		return false;
	}

	bool validateVertexElement(const PLY_ELEMENT &e, string *pErr)
	{
		bool bX = false;
		bool bY = false;
		bool bZ = false;

		for (const PLY_PROPERTY &p : e.m_vProp)
		{
			IF_CONT(p.m_bList);

			string n = toLower(p.m_name);
			if (n == "x")
				bX = true;
			else if (n == "y")
				bY = true;
			else if (n == "z")
				bZ = true;
		}

		if (!bX || !bY || !bZ)
		{
			if (pErr)
				*pErr = "PLY vertex element does not contain x, y, z properties";
			return false;
		}

		return true;
	}

	bool readVertexElement(istream &is,
						   const PLY_ELEMENT &e,
						   PLY_FORMAT format,
						   vector<kai::GEOMETRY_POINT> *pVP,
						   string *pErr)
	{
		NULL_F(pVP);
		IF_F(!validateVertexElement(e, pErr));

		if (e.m_n <= (uint64_t)(pVP->max_size() - pVP->size()))
			pVP->reserve(pVP->size() + (size_t)e.m_n);

		for (uint64_t i = 0; i < e.m_n; i++)
		{
			kai::GEOMETRY_POINT pt;
			pt.m_vP.setZero();
			pt.m_vC.setOnes();
			pt.m_tStamp = 1;

			for (const PLY_PROPERTY &p : e.m_vProp)
			{
				if (p.m_bList)
				{
					IF_F(!skipProperty(is, p, format, pErr));
					continue;
				}

				PLY_VALUE v;
				IF_F(!readValue(is, p.m_type, format, &v, pErr));
				setVertexScalar(p, v, &pt);
			}

			pVP->push_back(pt);
		}

		return true;
	}

	bool skipElement(istream &is, const PLY_ELEMENT &e, PLY_FORMAT format, string *pErr)
	{
		for (uint64_t i = 0; i < e.m_n; i++)
		{
			for (const PLY_PROPERTY &p : e.m_vProp)
				IF_F(!skipProperty(is, p, format, pErr));
		}

		return true;
	}

	bool readPlyHeader(istream &is,
					   PLY_FORMAT *pFormat,
					   vector<PLY_ELEMENT> *pVElem,
					   string *pErr)
	{
		NULL_F(pFormat);
		NULL_F(pVElem);

		string line;
		if (!getline(is, line))
		{
			if (pErr)
				*pErr = "Empty PLY file";
			return false;
		}

		trimLineEnd(&line);
		if (line != "ply")
		{
			if (pErr)
				*pErr = "Missing PLY magic header";
			return false;
		}

		*pFormat = ply_format_unknown;
		pVElem->clear();
		bool bEndHeader = false;

		while (getline(is, line))
		{
			trimLineEnd(&line);
			stringstream ss(line);
			string cmd;
			ss >> cmd;
			cmd = toLower(cmd);

			if (cmd.empty() || cmd == "comment" || cmd == "obj_info")
				continue;

			if (cmd == "end_header")
			{
				bEndHeader = true;
				break;
			}

			if (cmd == "format")
			{
				string fmt;
				string version;
				ss >> fmt >> version;
				fmt = toLower(fmt);

				if (fmt == "ascii")
					*pFormat = ply_format_ascii;
				else if (fmt == "binary_little_endian")
					*pFormat = ply_format_binary_little_endian;
				else if (fmt == "binary_big_endian")
					*pFormat = ply_format_binary_big_endian;
				else
				{
					if (pErr)
						*pErr = "Unsupported PLY format: " + fmt;
					return false;
				}

				continue;
			}

			if (cmd == "element")
			{
				PLY_ELEMENT e;
				ss >> e.m_name >> e.m_n;
				e.m_name = toLower(e.m_name);

				if (e.m_name.empty() || ss.fail())
				{
					if (pErr)
						*pErr = "Invalid PLY element declaration";
					return false;
				}

				pVElem->push_back(e);
				continue;
			}

			if (cmd == "property")
			{
				if (pVElem->empty())
				{
					if (pErr)
						*pErr = "PLY property appears before any element";
					return false;
				}

				PLY_PROPERTY p;
				string type;
				ss >> type;
				type = toLower(type);

				if (type == "list")
				{
					string typeCount;
					string typeItem;
					ss >> typeCount >> typeItem >> p.m_name;

					p.m_bList = true;
					p.m_typeCount = parseType(typeCount);
					p.m_typeItem = parseType(typeItem);
					if (p.m_name.empty() ||
						p.m_typeCount == ply_type_invalid ||
						p.m_typeItem == ply_type_invalid)
					{
						if (pErr)
							*pErr = "Invalid PLY list property declaration";
						return false;
					}
				}
				else
				{
					ss >> p.m_name;
					p.m_bList = false;
					p.m_type = parseType(type);
					if (p.m_name.empty() || p.m_type == ply_type_invalid)
					{
						if (pErr)
							*pErr = "Invalid PLY scalar property declaration";
						return false;
					}
				}

				p.m_name = toLower(p.m_name);
				pVElem->back().m_vProp.push_back(p);
				continue;
			}
		}

		if (!bEndHeader)
		{
			if (pErr)
				*pErr = "PLY header ended before end_header";
			return false;
		}

		if (*pFormat == ply_format_unknown)
		{
			if (pErr)
				*pErr = "PLY header has no format declaration";
			return false;
		}

		return true;
	}

	bool readPlyFile(const string &fName,
					 vector<kai::GEOMETRY_POINT> *pVP,
					 string *pErr)
	{
		NULL_F(pVP);
		pVP->clear();

		ifstream is(fName, ios::binary);
		if (!is.is_open())
		{
			if (pErr)
				*pErr = "Cannot open file";
			return false;
		}

		PLY_FORMAT format = ply_format_unknown;
		vector<PLY_ELEMENT> vElem;
		IF_F(!readPlyHeader(is, &format, &vElem, pErr));

		for (const PLY_ELEMENT &e : vElem)
		{
			if (e.m_name == "vertex")
				return readVertexElement(is, e, format, pVP, pErr);

			IF_F(!skipElement(is, e, format, pErr));
		}

		if (pErr)
			*pErr = "PLY file has no vertex element";

		return false;
	}
}

namespace kai
{

	_PCfile::_PCfile()
	{
	}

	_PCfile::~_PCfile()
	{
	}

	bool _PCfile::savePLY(const string &path, const vector<Vector3f> &points,
		const vector<Vector3f> &colors, string *error)
	{
		static_assert(sizeof(float) == 4 && std::numeric_limits<float>::is_iec559,
			"PLY output requires IEEE 754 float32");
		if (error) error->clear();
		auto fail = [&](const string &message) {
			if (error) *error = message;
			return false;
		};
		auto ioError = [&](const string &operation) {
			const int code = errno;
			return fail(operation + ": " + std::strerror(code));
		};
		try
		{
			const std::filesystem::path destination(path);
			if (path.empty() || path.find('\0') != string::npos || destination.filename().empty() ||
				destination.filename() == "." || destination.filename() == "..")
				return fail("Invalid PLY destination path");
			for (size_t i = 0; i < points.size(); ++i)
				if (!std::isfinite(points[i].x()) || !std::isfinite(points[i].y()) || !std::isfinite(points[i].z()))
					return fail("Nonfinite PLY point at index " + std::to_string(i));

			// Keep the temporary file beside the destination so rename is atomic.
			PLY_OUTPUT_FILE output;
			output.path = (destination.parent_path() / ".openkai-ply-XXXXXX").string();
			output.descriptor = mkstemp(output.path.data());
			if (output.descriptor < 0) return ioError("Cannot create temporary PLY file");
			output.created = true;
			output.stream = fdopen(output.descriptor, "wb");
			if (!output.stream) return ioError("Cannot open temporary PLY stream");
			const string header = "ply\nformat binary_little_endian 1.0\nelement vertex " + std::to_string(points.size()) +
				"\nproperty float x\nproperty float y\nproperty float z\nproperty uchar red\nproperty uchar green\nproperty uchar blue\nend_header\n";
			if (fwrite(header.data(), 1, header.size(), output.stream) != header.size())
				return ioError("Cannot write PLY header");

			// Pack blocks without struct padding or dependence on host endianness.
			constexpr size_t vertexBytes = 15;
			std::array<uint8_t, vertexBytes * 4096> buffer;
			size_t used = 0;
			for (size_t i = 0; i < points.size(); ++i)
			{
				for (int axis = 0; axis < 3; ++axis)
				{
					uint32_t bits;
					const float value = points[i][axis];
					std::memcpy(&bits, &value, sizeof(bits));
					for (int byte = 0; byte < 4; ++byte)
						buffer[used++] = uint8_t(bits >> (8 * byte));
				}
				for (int channel = 0; channel < 3; ++channel)
				{
					const float value = i < colors.size() ? colors[i][channel] : 1.0f;
					buffer[used++] = std::isfinite(value) ? uint8_t(std::clamp(value, 0.0f, 1.0f) * 255.0f + 0.5f) : 255;
				}
				if (used == buffer.size())
				{
					if (fwrite(buffer.data(), 1, used, output.stream) != used) return ioError("Cannot write PLY vertices");
					used = 0;
				}
			}
			if (used && fwrite(buffer.data(), 1, used, output.stream) != used) return ioError("Cannot write PLY vertices");
			if (fflush(output.stream) != 0) return ioError("Cannot flush PLY file");
			if (fsync(output.descriptor) != 0) return ioError("Cannot synchronize PLY file");
			FILE *stream = output.stream;
			output.stream = nullptr;
			output.descriptor = -1; // fclose closes the descriptor even on failure.
			if (fclose(stream) != 0) return ioError("Cannot close PLY file");
			if (std::rename(output.path.c_str(), path.c_str()) != 0) return ioError("Cannot replace PLY destination");
			output.created = false;
			return true;
		}
		catch (const std::exception &e)
		{
			return fail(string("Cannot save PLY: ") + e.what());
		}
	}

	bool _PCfile::loadConfig(void)
	{
		IF_F(!this->_PointCloud::loadConfig());
		const json &j = *m_pJ;

		jKv(j, "vfName", m_vfName);
		open();

		return true;
	}

	bool _PCfile::saveConfig(bool bExport)
	{
		IF_F(!_PointCloud::saveConfig(false));

		json &j = *m_pJ;
		j["vfName"] = m_vfName;

		IF__(!bExport, true);
		return m_pJcfg->saveToFile();
	}

	bool _PCfile::open(void)
	{
		IF_F(m_vfName.empty());

		vector<GEOMETRY_POINT> vPoint;
		for (string f : m_vfName)
		{
			vector<GEOMETRY_POINT> vPointFile;
			string err;
			if (!readPlyFile(f, &vPointFile, &err))
			{
				LOG_E("PLY read failed: " + f + ", " + err);
				continue;
			}

			vPoint.insert(vPoint.end(), vPointFile.begin(), vPointFile.end());
			LOG_I("File: " + f + ", Npoints: " + u642str(vPointFile.size()));
		}

		clear();

		for (GEOMETRY_POINT p : vPoint)
		{
			p.m_vP = m_mPosef * p.m_vP;
			m_grPt.add(p);
		}

		return true;
	}

	bool _PCfile::start(void)
	{
		NULL_F(m_pT);
		return m_pT->startThread(getUpdate, this);
	}

	void _PCfile::update(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPS();
		}
	}

}
