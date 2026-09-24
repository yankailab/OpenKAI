#include "../../src/Universe/Geometry/PointCloud/_PCfile.h"
#include <cassert>
#include <csignal>
#include <filesystem>
#include <fstream>
#include <limits>
#include <sys/resource.h>
#include <sys/wait.h>
#include <unistd.h>

using namespace kai;
namespace fs = std::filesystem;

std::string contents(const fs::path &path)
{
	std::ifstream input(path, std::ios::binary);
	assert(input);
	return {std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};
}

int main()
{
	std::string directory = (fs::temp_directory_path() / "openkai-pcfile-test-XXXXXX").string();
	assert(mkdtemp(directory.data()));
	const fs::path root(directory), path = root / "cloud.ply";
	const vector<Vector3f> points{{1, -2, .25f}, {4.5f, 6, -8}, {9, 10, 11}};
	const vector<Vector3f> colors{{0, .5f, 1}, {-1, 2, std::numeric_limits<float>::quiet_NaN()}};
	std::string error = "old error";
	assert(_PCfile::savePLY(path.string(), points, colors, &error) && error.empty());
	const std::string saved = contents(path);
	const size_t headerEnd = saved.find("end_header\n") + std::string("end_header\n").size();
	assert(saved.find("ply\nformat binary_little_endian 1.0\nelement vertex 3\n") == 0);
	assert(headerEnd < saved.size() && saved.size() - headerEnd == points.size() * 15);
	const auto *bytes = reinterpret_cast<const unsigned char *>(saved.data() + headerEnd);
	assert(bytes[0] == 0 && bytes[1] == 0 && bytes[2] == 128 && bytes[3] == 63); // 1.0f, little endian
	assert(bytes[12] == 0 && bytes[13] == 128 && bytes[14] == 255);
	assert(bytes[27] == 0 && bytes[28] == 255 && bytes[29] == 255);
	assert(bytes[42] == 255 && bytes[43] == 255 && bytes[44] == 255);

	// Roundtrip through the existing PLY reader, including 8-bit color precision.
	_PCfile loaded;
	const json config = {{"name", "loaded"}, {"class", "_PCfile"}, {"nP", 3}, {"vfName", {path.string()}},
		{"thread", {{"name", "thread"}, {"class", "_Thread"}, {"FPS", 1}}}};
	assert(loaded.init(config));
	GEOMETRY_RINGBUF<GEOMETRY_POINT> ring;
	assert(ring.alloc(3));
	assert(loaded.get(&ring) == 3);
	for (int i = 0; i < 3; ++i) assert(ring.get(i)->m_vP == points[2 - i]);
	assert(ring.get(2)->m_vC.isApprox(Vector3f(0, 128.0f / 255, 1), 1e-6));
	assert(ring.get(1)->m_vC.isApprox(Vector3f(0, 1, 1), 1e-6));
	assert(ring.get(0)->m_vC.isOnes());
	ring.release();

	// Exercise both a complete output block and its trailing partial block.
	const fs::path blocksPath = root / "blocks.ply";
	assert(_PCfile::savePLY(blocksPath.string(), vector<Vector3f>(5001, points[0])));
	const std::string blocks = contents(blocksPath);
	const size_t blocksStart = blocks.find("end_header\n") + 11;
	assert(blocks.size() - blocksStart == 5001 * 15);
	for (size_t i = 0; i < 5001; ++i)
	{
		assert(blocks.compare(blocksStart + i * 15, 12, saved, headerEnd, 12) == 0);
		for (size_t channel = 12; channel < 15; ++channel)
			assert(static_cast<unsigned char>(blocks[blocksStart + i * 15 + channel]) == 255);
	}
	fs::remove(blocksPath);

	// Invalid coordinates must leave an existing valid file untouched.
	for (float invalid : {std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::infinity()})
	{
		assert(!_PCfile::savePLY(path.string(), {Vector3f(invalid, 0, 0)}, {}, &error));
		assert(!error.empty() && contents(path) == saved);
	}
	assert(!_PCfile::savePLY("", points, {}, &error) && !error.empty());
	assert(!_PCfile::savePLY((root / "missing" / "cloud.ply").string(), points, {}, &error) && !error.empty());
	assert(!fs::exists(root / "missing"));
	assert(!_PCfile::savePLY(root.string(), points, {}, &error) && !error.empty());
	assert(fs::is_directory(root));

	// A small stdio write can succeed yet fail at fflush. Exercise that path
	// without changing the test process's file-size limit or signal handler.
	const pid_t child = fork();
	assert(child >= 0);
	if (child == 0)
	{
		std::signal(SIGXFSZ, SIG_IGN);
		const rlimit limit{0, 0};
		if (setrlimit(RLIMIT_FSIZE, &limit) != 0) _exit(2);
		const bool failed = !_PCfile::savePLY(path.string(), points, colors, &error);
		_exit(failed && error.find("Cannot flush PLY file") == 0 ? 0 : 1);
	}
	int status;
	assert(waitpid(child, &status, 0) == child && WIFEXITED(status) && WEXITSTATUS(status) == 0);
	assert(contents(path) == saved);
	for (const auto &entry : fs::directory_iterator(root)) assert(entry.path() == path);

	// Empty point clouds are valid PLY files, and successful replacement truncates.
	assert(_PCfile::savePLY(path.string(), {}, {}, &error) && error.empty());
	const std::string empty = contents(path);
	assert(empty.find("element vertex 0\n") != std::string::npos);
	assert(empty.substr(empty.size() - 11) == "end_header\n");
	_PCfile emptyLoaded;
	assert(emptyLoaded.init(config));
	assert(ring.alloc(1));
	assert(emptyLoaded.get(&ring) == 0);
	ring.release();
	fs::remove_all(root);
	return 0;
}
