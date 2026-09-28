#include "../../src/Base/_ModuleBase.h"
#include "../../src/UI/_Console.h"
#include "../../src/IO/_IObase.h"
#include "../../src/DataStream/LineFrame.h"
#include "../../src/Universe/Geometry/PointCloud/_PointCloud.h"
#include "../../src/Universe/Geometry/Line/_Line.h"
#include <cassert>
#include <chrono>
#include <cstdio>
#include <thread>

// Only expose pipeline entry points in this standalone translation unit.
#define private public
#include "../../src/Universe/Geometry/PointCloud/Pipeline/_PCtransform.h"
#include "../../src/Universe/Geometry/PointCloud/Pipeline/_PCmerge.h"
#include "../../src/Universe/Geometry/PointCloud/Pipeline/_PCsend.h"
#include "../../src/Universe/Geometry/PointCloud/Pipeline/_PCrecv.h"
#include "../../src/Universe/Geometry/PointCloud/_PCfile.h"
#undef private

using namespace kai;

namespace
{
	std::map<string, DataStreamBase *> streams;

	class Transform : public _PCtransform
	{
	public:
		Transform(PCLframe &input, PCLframe &output)
		{
			m_pPCLin = &input;
			m_pPCL = &output;
			m_pT = new _Thread();
		}

		void expireAfter(uint64_t duration)
		{
			m_dTexpire = duration;
			m_bTransformChanged = true;
		}
	};

	class Merge : public _PCmerge
	{
	public:
		Merge(PCLframe &first, PCLframe &second, PCLframe &output, float voxelSize = 0.0f)
		{
			m_vpPCL = {&first, &second};
			m_vInputRevision = {0, 0};
			m_pPCL = &output;
			m_rVoxel = voxelSize;
			m_pT = new _Thread();
		}
	};

	class MemoryIO : public _IObase
	{
	public:
		bool bOpen(void) override
		{
			return true;
		}

		bool write(uint8_t *pBytes, int count) override
		{
			m_vPackets.emplace_back(pBytes, pBytes + count);
			return true;
		}

		vector<vector<uint8_t>> m_vPackets;
	};

	class Sender : public _PCsend
	{
	public:
		Sender(PCLframe &input, MemoryIO &io)
		{
			m_pPCLin = &input;
			m_pIO = &io;
			m_vPacket.resize(96);
			m_pT = new _Thread();
			m_pT->run();
			m_tInt = 0;
		}
	};

	class Receiver : public _PCrecv
	{
	public:
		explicit Receiver(PCLframe &output)
		{
			m_pPCL = &output;
		}

		void feed(const vector<uint8_t> &bytes)
		{
			for (uint8_t byte : bytes)
			{
				inputByte(byte);
			}
		}
	};

	class File : public _PCfile
	{
	public:
		File(PCLframe &output, const string &path)
		{
			m_pPCL = &output;
			m_vfName = {path};
		}
	};

	GEOMETRY_POINT point(float x, uint64_t stamp = 100)
	{
		GEOMETRY_POINT value;
		value.m_vP = Vector3f(x, 2.0f, 3.0f);
		value.m_vC = Vector3f(0.1f, 0.5f, 0.9f);
		value.m_tStamp = stamp;
		return value;
	}

	void testTransform(void)
	{
		PCLframe input;
		PCLframe output;
		Transform transform(input, output);
		transform.setPos(1.0, 0.0, 0.0);
		Eigen::Matrix4d translation = Eigen::Matrix4d::Identity();
		translation(0, 3) = 4.0;
		transform.setTranslationMatrix(translation);
		input.set({point(2), point(7, 0)}, 100);
		transform.updateTransform();
		const auto first = output.get();
		assert(first->m_vPoints.size() == 1);
		assert(first->m_vPoints[0].m_vP.x() == 7.0f);
		assert(first->m_tStamp == 100);
		transform.updateTransform();
		assert(output.get() == first);

		input.set({point(8)}, 1);
		transform.updateTransform();
		assert(output.get()->m_vPoints[0].m_vP.x() == 13.0f);
		assert(output.get()->m_tStamp == 1);
		input.set({}, 1);
		transform.updateTransform();
		assert(output.get()->m_vPoints.empty());

		transform.expireAfter(5 * NSEC_MSEC);
		input.set({point(1, getTns())}, 2);
		transform.updateTransform();
		assert(output.get()->m_vPoints.size() == 1);
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		transform.updateTransform();
		assert(output.get()->m_vPoints.empty());
	}

	void testMerge(void)
	{
		PCLframe first;
		PCLframe second;
		PCLframe output;
		Merge merge(first, second, output);
		first.set({point(1), point(2)}, 30);
		second.set({point(3)}, 20);
		merge.updateMerge();
		assert(output.get()->m_vPoints.size() == 3);
		assert(output.get()->m_tStamp == 30);
		const auto snapshot = output.get();
		merge.updateMerge();
		assert(output.get() == snapshot);
		first.set({}, 30);
		merge.updateMerge();
		assert(output.get()->m_vPoints.size() == 1);
		assert(output.get()->m_vPoints[0].m_vP.x() == 3);
		second.set({}, 1);
		merge.updateMerge();
		assert(output.get()->m_vPoints.empty());

		Merge voxels(first, second, output, 1.0f);
		first.set({point(0.2f)}, 3);
		second.set({point(0.4f), point(2.0f)}, 3);
		voxels.updateMerge();
		assert(output.get()->m_vPoints.size() == 2);
		assert(std::abs(output.get()->m_vPoints[0].m_vP.x() - 0.3f) < 1.0e-6f);
	}

	void testProtocol(void)
	{
		PCLframe input;
		PCLframe output;
		MemoryIO io;
		Sender sender(input, io);
		Receiver receiver(output);
		input.set({point(1000), point(-700), point(0.125f)}, 99);
		sender.sendPC();
		assert(io.m_vPackets.size() == 2);
		receiver.feed(io.m_vPackets[0]);
		assert(output.get()->m_revision == 0);
		receiver.feed(io.m_vPackets[1]);
		const auto complete = output.get();
		assert(complete->m_vPoints.size() == 3);
		assert(complete->m_vPoints[0].m_vP.x() == 1000.0f);
		assert(complete->m_vPoints[1].m_vP.x() == -700.0f);
		assert(complete->m_vPoints[2].m_vP.x() == 0.125f);
		assert(complete->m_vPoints[0].m_tStamp == 100);
		assert(complete->m_tStamp == 99);
		sender.sendPC();
		assert(io.m_vPackets.size() == 2);

		// Out-of-order continuation and malformed packet sizes cannot publish.
		receiver.feed(io.m_vPackets[1]);
		assert(output.get() == complete);
		vector<uint8_t> malformed = io.m_vPackets[0];
		pcstream::packUint(malformed.data() + 4, UINT32_MAX, 4);
		receiver.feed(malformed);
		assert(output.get() == complete);
		receiver.feed(io.m_vPackets[0]);
		receiver.feed(io.m_vPackets[1]);
		assert(output.get()->m_revision == complete->m_revision + 1);

		io.m_vPackets.clear();
		input.set({}, 1);
		sender.sendPC();
		assert(io.m_vPackets.size() == 1);
		assert(io.m_vPackets[0].size() == pcstream::headerBytes);
		receiver.feed(io.m_vPackets[0]);
		assert(output.get()->m_vPoints.empty());
		assert(output.get()->m_tStamp == 1);
	}

	void testFile(void)
	{
		char path[] = "/tmp/openkai-ply-stream-XXXXXX";
		const int descriptor = mkstemp(path);
		assert(descriptor >= 0);
		close(descriptor);
		const vector<GEOMETRY_POINT> points = {point(500), point(-8)};
		string error;
		assert(_PCfile::savePLY(path, points, &error));
		PCLframe output;
		File file(output, path);
		assert(file.open());
		const auto loaded = output.get();
		assert(loaded->m_vPoints.size() == points.size());
		for (size_t i = 0; i < points.size(); ++i)
		{
			assert(loaded->m_vPoints[i].m_vP == points[i].m_vP);
			assert((loaded->m_vPoints[i].m_vC - points[i].m_vC).cwiseAbs().maxCoeff() <= 1.0f / 255.0f);
		}
		std::remove(path);
		assert(!file.open());
		assert(output.get() == loaded);
	}
}

// Registry and console are the only application services replaced by the test.
namespace kai
{
	void _Console::addMsg(const string &, int)
	{
	}

	void _Console::addMsg(const string &, int, int, int)
	{
	}

	void *InstanceMgr::findDataStream(const string &name)
	{
		const auto found = streams.find(name);
		return found == streams.end() ? nullptr : found->second;
	}

	void *InstanceMgr::findModule(const string &)
	{
		return nullptr;
	}
}

int main(void)
{
	testTransform();
	testMerge();
	testProtocol();
	testFile();
	std::puts("Geometry stream tests passed");
	return 0;
}
