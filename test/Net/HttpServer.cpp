#include "../../src/Net/HttpServer.h"
#include "../../src/UI/Viewer/Web/WebAssets.h"
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/core.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <unistd.h>

namespace fs = std::filesystem;
namespace http = boost::beast::http;

static void require(bool value, const char *message)
{
    if (!value) throw std::runtime_error(message);
}

int main()
{
    const auto dir = fs::temp_directory_path() / ("openkai-http-test-" + std::to_string(getpid()));
    const auto webRoot = dir / "html/viewer/test";
    fs::create_directories(webRoot);
    fs::create_directories(dir / "html/common/css");
    fs::create_directories(dir / "html/common/js");
    fs::create_directories(dir / "html/common/img");
    fs::create_directories(dir / "standalone/isolated");
    fs::create_directories(dir / "models/drone");
    std::ofstream(webRoot / "index.html") << "viewer";
    std::ofstream(webRoot / "models-other") << "ordinary root file";
    std::ofstream(webRoot / "common-other") << "local asset";
    std::ofstream(dir / "html/common/css/theme.css") << "body { color: white; }";
    std::ofstream(dir / "html/common/js/theme.js") << "export const theme = 'dark';";
    std::ofstream(dir / "html/common/img/icon.png") << "png-test";
    std::ofstream(dir / "models/drone/quad.glb") << "glb-test";
    std::ofstream(dir / "models/decoder.wasm") << "wasm-test";
    std::ofstream(dir / "private.txt") << "outside both roots";
    fs::create_symlink(dir / "private.txt", dir / "models/escape.txt");
    fs::create_symlink(dir / "private.txt", dir / "html/common/escape.txt");
    fs::create_symlink(dir / "models/drone/quad.glb", webRoot / "escape.glb");
    try
    {
        kai::HttpServer server;
        std::string error;
        auto mounts = kai::webviewer::commonAssets(webRoot.string() + "/");
        require(mounts.size() == 1 && mounts.front().first == "/common", "shared viewer assets not located");
        require(kai::webviewer::commonAssets((dir / "standalone/isolated").string()).empty(),
                "standalone web root requires shared assets");
        mounts.emplace_back("/models", (dir / "models").string());
        const bool started = server.start("127.0.0.1", 0, webRoot.string(), {}, &error, 32, mounts);
        require(started, error.c_str());
        auto get = [&](std::string target, http::verb method = http::verb::get) {
            boost::asio::io_context io;
            boost::beast::tcp_stream stream(io);
            stream.connect({boost::asio::ip::make_address("127.0.0.1"), server.port()});
            http::request<http::empty_body> request{method, target, 11};
            request.set(http::field::host, "localhost");
            http::write(stream, request);
            boost::beast::flat_buffer buffer;
            http::response_parser<http::string_body> parser;
            if (method == http::verb::head) parser.skip(true);
            http::read(stream, buffer, parser);
            return parser.release();
        };
        require(get("/").body() == "viewer", "default web root changed");
        const auto css = get("/common/css/theme.css");
        require(css.result() == http::status::ok && css.body() == "body { color: white; }", "shared CSS mount failed");
        require(css[http::field::content_type] == "text/css; charset=utf-8", "CSS MIME incorrect");
        require(get("/common/js/theme.js").body() == "export const theme = 'dark';", "shared JavaScript mount failed");
        require(get("/common/img/icon.png")[http::field::content_type] == "image/png", "shared image MIME incorrect");
        require(get("/common-other").body() == "local asset", "shared mount prefix boundary failed");
        require(get("/private.txt").result() == http::status::not_found, "shared assets widened web root");
        const auto model = get("/models/drone/quad.glb?v=1");
        require(model.result() == http::status::ok && model.body() == "glb-test", "model mount failed");
        require(model[http::field::content_type] == "model/gltf-binary", "GLB MIME incorrect");
        require(get("/models/decoder.wasm")[http::field::content_type] == "application/wasm", "WASM MIME incorrect");
        const auto head = get("/models/drone/quad.glb", http::verb::head);
        require(head.result() == http::status::ok && head.body().empty() && head[http::field::content_length] == "8", "HEAD failed");
        require(get("/models-other").body() == "ordinary root file", "mount prefix boundary failed");
        for (const auto *target : {"/models/../private.txt", "/models/%2e%2e/private.txt",
                                   "/models/%2e%2e%2fprivate.txt", "/models/escape.txt", "/escape.glb",
                                   "/common/../viewer/test/index.html", "/common/%2e%2e/viewer/test/index.html",
                                   "/common/escape.txt"})
            require(get(target).result() == http::status::forbidden, "path escaped configured root");
        require(get("/models/%00").result() == http::status::bad_request, "invalid encoded path accepted");
        require(get("/models/missing").result() == http::status::not_found, "missing asset response incorrect");
        require(get("/models/drone/quad.glb", http::verb::post).result() == http::status::method_not_allowed, "mount accepted write");
        server.stop();
        kai::HttpServer invalid;
        require(!invalid.start("127.0.0.1", 0, webRoot.string(), {}, &error, 32,
                               {{"/models/..", (dir / "models").string()}}), "invalid mount accepted");
        fs::remove_all(dir);
        std::cout << "HTTP root, mounted assets, MIME, HEAD and traversal tests passed\n";
    }
    catch (const std::exception &error)
    {
        fs::remove_all(dir);
        std::cerr << error.what() << '\n';
        return 1;
    }
}
