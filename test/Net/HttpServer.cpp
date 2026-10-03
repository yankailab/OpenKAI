#include "../../src/Net/HttpServer.h"
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
    fs::create_directories(dir / "web");
    fs::create_directories(dir / "models/drone");
    std::ofstream(dir / "web/index.html") << "viewer";
    std::ofstream(dir / "web/models-other") << "ordinary root file";
    std::ofstream(dir / "models/drone/quad.glb") << "glb-test";
    std::ofstream(dir / "models/decoder.wasm") << "wasm-test";
    std::ofstream(dir / "private.txt") << "outside both roots";
    fs::create_symlink(dir / "private.txt", dir / "models/escape.txt");
    fs::create_symlink(dir / "models/drone/quad.glb", dir / "web/escape.glb");
    try
    {
        kai::HttpServer server;
        std::string error;
        const bool started = server.start("127.0.0.1", 0, (dir / "web").string(), {}, &error, 32,
                                          {{"/models", (dir / "models").string()}});
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
        const auto model = get("/models/drone/quad.glb?v=1");
        require(model.result() == http::status::ok && model.body() == "glb-test", "model mount failed");
        require(model[http::field::content_type] == "model/gltf-binary", "GLB MIME incorrect");
        require(get("/models/decoder.wasm")[http::field::content_type] == "application/wasm", "WASM MIME incorrect");
        const auto head = get("/models/drone/quad.glb", http::verb::head);
        require(head.result() == http::status::ok && head.body().empty() && head[http::field::content_length] == "8", "HEAD failed");
        require(get("/models-other").body() == "ordinary root file", "mount prefix boundary failed");
        for (const auto *target : {"/models/../private.txt", "/models/%2e%2e/private.txt",
                                   "/models/%2e%2e%2fprivate.txt", "/models/escape.txt", "/escape.glb"})
            require(get(target).result() == http::status::forbidden, "path escaped configured root");
        require(get("/models/%00").result() == http::status::bad_request, "invalid encoded path accepted");
        require(get("/models/missing").result() == http::status::not_found, "missing asset response incorrect");
        require(get("/models/drone/quad.glb", http::verb::post).result() == http::status::method_not_allowed, "mount accepted write");
        server.stop();
        kai::HttpServer invalid;
        require(!invalid.start("127.0.0.1", 0, (dir / "web").string(), {}, &error, 32,
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
