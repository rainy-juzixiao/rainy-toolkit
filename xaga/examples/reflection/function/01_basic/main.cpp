#include <rainy/collections/vector.hpp>
#include <rainy/core/core.hpp>
#include <rainy/foundation/concurrency/concurrency.hpp>
#include <rainy/foundation/io/net/ssl/stream.hpp>
#include <rainy/foundation/io/net/timer.hpp>

#include "rainy/foundation/io/net/internet.hpp"
#if RAINY_USING_WINDOWS
#include <windows.h>
#endif
#include <iostream>
using namespace rainy;
int main() {
    {
        rainy::foundation::io::net::io_context io_ctx;
        rainy::foundation::io::net::ssl::context ctx(rainy::foundation::io::net::ssl::method::tls_client);

        // 2. 配置证书（使用系统证书库验证服务器）
        rainy::foundation::io::net::ssl::certificate_options cert_opts;
        cert_opts.use_system_store(); // Windows/Linux/macOS自动使用系统证书
        ctx.load_certificate(cert_opts);
        ctx.set_verify_mode(rainy::foundation::io::net::ssl::verify_mode::peer);

        // 3. 创建TCP连接
        rainy::foundation::io::net::ip::tcp::socket sock(io_ctx);
        rainy::foundation::io::net::ip::tcp::resolver resolver(io_ctx);
        auto endpoints = resolver.resolve("www.baidu.com", "443");
        sock.connect(*endpoints.begin());

        // 4. 创建SSL流
        rainy::foundation::io::net::ssl::ssl_stream<decltype(sock)> ssl_stream(std::move(sock), ctx);

        // 5. 设置服务器名称（SNI）
        ssl_stream.set_server_name("www.baidu.com");

        // 6. SSL握手
        ssl_stream.handshake();
        std::cout << "SSL握手成功！" << std::endl;

        // 7. 发送HTTP请求
        foundation::text::string request = "GET / HTTP/1.1\r\n"
                                           "Host: www.baidu.com\r\n"
                                           "Connection: close\r\n"
                                           "\r\n";
        ssl_stream.write_some(foundation::io::net::buffer(request));

        // 8. 读取响应
        char buffer[4096];
        std::error_code ec;

        for (;;) {
            auto bytes = ssl_stream.read_some(foundation::io::net::buffer(buffer), ec);

            if (ec) {
                throw std::system_error(ec);
            }

            if (bytes == 0) {
                break; // EOF
            }

            std::cout.write(buffer, bytes);
        }
        // 9. 关闭SSL连接
        ssl_stream.shutdown(ec);

        if (ec && ec != std::errc::not_connected && ec != std::errc::connection_reset) {
            throw std::system_error(ec, "SSL shutdown failed");
        }
    }
    rainy::foundation::io::net::io_context ctx;
    rainy::foundation::io::net::steady_timer timer(ctx, std::chrono::seconds{5});
    timer.async_wait([](std::error_code ec) {
        if (!ec) {
            std::cout << "!!!\n";
        }
    });
    timer.async_wait([](std::error_code ec) {
        if (!ec) {
            std::cout << "!!!!\n";
        }
    });
    ctx.run();
#if RAINY_USING_WINDOWS
    SetConsoleOutputCP(CP_UTF8);
#endif
    try {
        int number = 1234567;
        std::string str = "Standard String";
        std::cout << "en_US: "
                  << foundation::text::format(std::locale("en_US.UTF-8"), "Hello World {} {:L} {} {} {} {0}", 3.14, number,
                                              "This char array", str, &str, nullptr)
                  << std::endl; // 1,234,567 Hello World This char array Standard String 0xdaa12ff438 0x0
        std::cout << "de_DE: " << foundation::text::format(std::locale("de_DE.UTF-8"), "{:L}", number) << std::endl; // 1.234.567
        std::cout << "fr_FR: "
                  << foundation::text::format(std::locale("fr_FR.UTF-8"), "{:L} Hello World 诗人握持 {}", number, "诗人握持")
                  << std::endl; // 1 234 567
        std::cout << "swiss: " << foundation::text::format(std::locale("de_CH.UTF-8"), "{:L}", number) << std::endl; // 1'234'567
    } catch (std::exception &e) {
        std::cout << e.what() << '\n';
    }
    char buf[200]{};
    foundation::text::to_chars(buf, buf + 10, 11111);
    std::cout << buf << '\n';

    /*rainy::foundation::concurrency::atomic<float> atomics = 10;
    std::cout << atomics << '\n';
    std::cout << atomics + 3.14f << '\n';
    std::cout << atomics + 3.14 << '\n';
    std::cout << atomics.fetch_sub(10) << '\n';
    std::cout << atomics << '\n';
    std::cout << atomics.fetch_sub(10) << '\n';
    rainy::foundation::concurrency::atomic_fetch_add(&atomics, 100);
    std::cout << atomics << '\n';*/
    return 0;
}
