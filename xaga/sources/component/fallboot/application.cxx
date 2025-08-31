#include <rainy/component/fallboot/applocation.hpp>
#include <WS2spi.h>
#include <windows.h>
#include <iostream>
#pragma comment(lib, "ws2_32.lib")

namespace rainy::component::fallboot::implements {
    std::once_flag &once_flag() noexcept {
        static std::once_flag flags;
        return flags;
    }

    class HttpServer {
    public:
        HttpServer() = default;

        HttpServer(unsigned short port) : port_(port), running_(false), serverSocket_(INVALID_SOCKET) {
        }

        void bind_port(unsigned short port) {
            this->port_ = port;
        }

        bool start() {
            if (WSAStartup(MAKEWORD(2, 2), &wsaData_) != 0) {
                std::cerr << "WSAStartup failed\n";
                return false;
            }

            serverSocket_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            if (serverSocket_ == INVALID_SOCKET) {
                std::cerr << "Socket creation failed\n";
                WSACleanup();
                return false;
            }

            sockaddr_in serverAddr{};
            serverAddr.sin_family = AF_INET;
            serverAddr.sin_addr.s_addr = INADDR_ANY;
            serverAddr.sin_port = htons(port_);

            if (bind(serverSocket_, (SOCKADDR *) &serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
                std::cerr << "Bind failed\n";
                closesocket(serverSocket_);
                WSACleanup();
                return false;
            }

            if (listen(serverSocket_, SOMAXCONN) == SOCKET_ERROR) {
                std::cerr << "Listen failed\n";
                closesocket(serverSocket_);
                WSACleanup();
                return false;
            }

            running_ = true;
            serverThread_ = std::thread(&HttpServer::run, this);

            std::cout << "Server started on port " << port_ << "\n";
            return true;
        }

        void stop() {
            if (!running_)
                return;

            running_ = false;

            // 触发 accept 返回
            SOCKET tmp = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            sockaddr_in loopback{};
            loopback.sin_family = AF_INET;
            loopback.sin_addr.s_addr = inet_addr("127.0.0.1");
            loopback.sin_port = htons(port_);
            connect(tmp, (sockaddr *) &loopback, sizeof(loopback));
            closesocket(tmp);

            if (serverThread_.joinable()) {
                serverThread_.join();
            }

            closesocket(serverSocket_);
            WSACleanup();
            std::cout << "Server stopped\n";
        }

    private:
        void run() {
            sockaddr_in clientAddr{};
            int clientAddrSize = sizeof(clientAddr);

            while (running_) {
                SOCKET clientSocket = accept(serverSocket_, (SOCKADDR *) &clientAddr, &clientAddrSize);
                if (!running_ || clientSocket == INVALID_SOCKET) {
                    continue;
                }

                std::thread(&HttpServer::handleClient, this, clientSocket).detach();
            }
        }

        void handleClient(SOCKET clientSocket) {
            const char *RESPONSE = "HTTP/1.1 200 OK\r\n"
                                   "Content-Type: text/plain\r\n"
                                   "Content-Length: 13\r\n"
                                   "\r\n"
                                   "Hello, World!";

            char buffer[1024] = {};
            int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
            if (bytesReceived > 0) {
                send(clientSocket, RESPONSE, strlen(RESPONSE), 0);
            }

            closesocket(clientSocket);
        }

        WSADATA wsaData_;
        unsigned short port_;
        SOCKET serverSocket_;
        std::atomic<bool> running_;
        std::thread serverThread_;
    };

    static HttpServer web_server_instance;
}

namespace rainy::component::fallboot {
    class webserver_application : public application {
    public:
        ~webserver_application() {
            this->stop_application();
        }

    private:
        void start_application() {
            implements::web_server_instance.start();
        }

        void stop_application() {
            implements::web_server_instance.stop();
        }

        void set_appname(std::string_view name) {
            this->app_name_ = name.data();
        }

        void set_config_path(std::filesystem::path config_path) {
            this->config_path_ = config_path;
        }

        text::string app_name_{"that-fallboot"};
        std::optional<std::filesystem::path> config_path_;
    };

    application &application::app_name(std::string_view name) {
        this->set_appname(name);
        return *this;
    }

    application &application::with_config_path(std::filesystem::path config_path) {
        this->set_config_path(config_path);
        return *this;
    }

    int application::run(application_type application_type, int argc, char *argv[]) {
        this->start_application();
        return 0;
    }

    static webserver_application web;

    application &application::init() {
        return web;
    }

    void application::shutdown() {
        
    }
}
