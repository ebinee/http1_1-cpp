#include <iostream>
#include <string>
#include <thread>
#include <sstream>
#include <cstring>
#include <vector>
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")

#ifndef socklen_t
typedef int socklen_t;
#endif

class MyHttpHandler {
public:
    struct HttpRequestInfo {
        std::string command;
        std::string path;
        std::string version;
        std::string request_line;
    };

    HttpRequestInfo parse_http_request(const std::string& request) {
        HttpRequestInfo info;
        std::istringstream iss(request);
        iss >> info.command >> info.path >> info.version;
        info.request_line = info.command + " " + info.path + " " + info.version;
        return info;
    }

    void print_http_request_detail(const std::string& client_ip, int client_port, const HttpRequestInfo& info) {
        std::cout << "::Client address   : " << client_ip << std::endl;
        std::cout << "::Client port      : " << client_port << std::endl;
        std::cout << "::Request command  : " << info.command << std::endl;
        std::cout << "::Request line     : " << info.request_line << std::endl;
        std::cout << "::Request path     : " << info.path << std::endl;
        std::cout << "::Request version  : " << info.version << std::endl;
    }

    std::string send_http_response_header() {
        std::ostringstream header;
        header << "HTTP/1.1 200 OK\r\n";
        header << "Content-Type: text/html\r\n";
        header << "Connection: close\r\n\r\n";
        return header.str();
    }

    void do_GET(int client_socket, const std::string& client_ip, int client_port, const HttpRequestInfo& info) {
        std::cout << "## do_GET() activated.\n";
        print_http_request_detail(client_ip, client_port, info);

        std::string header = send_http_response_header();
        std::ostringstream body;

        if (info.path.find('?') != std::string::npos) {
            std::string params = info.path.substr(info.path.find('?') + 1);
            auto vec = parameter_retrieval(params);
            int result = simple_calc(vec[0], vec[1]);

            body << "<html>";
            body << "GET request for calculation => " << vec[0] << " x " << vec[1] << " = " << result;
            body << "</html>";

            std::cout << "## GET request for calculation => " << vec[0] << " x " << vec[1] << " = " << result << ".\n";
        }
        else {
            body << "<html><p>HTTP Request GET for Path: " << info.path << "</p></html>";
            std::cout << "## GET request for directory => " << info.path << ".\n";
        }

        std::string response = header + body.str();
        send(client_socket, response.c_str(), response.size(), 0);
    }

    void do_POST(int client_socket, const std::string& client_ip, int client_port, const HttpRequestInfo& info, const std::string& body) {
        std::cout << "## do_POST() activated.\n";
        print_http_request_detail(client_ip, client_port, info);

        auto vec = parameter_retrieval(body);
        int result = simple_calc(vec[0], vec[1]);

        std::string header = send_http_response_header();
        std::ostringstream response_body;

        response_body << "POST request for calculation => "
            << vec[0] << " x " << vec[1] << " = " << result;

        std::cout << "## POST request data => " << body << ".\n";
        std::cout << "## POST request for calculation => "
            << vec[0] << " x " << vec[1] << " = " << result << ".\n";

        std::string response = header + response_body.str();
        send(client_socket, response.c_str(), response.size(), 0);
    }


    void log_message() {}

    int simple_calc(int para1, int para2) {
        return para1 * para2;
    }

    std::vector<int> parameter_retrieval(const std::string& msg) {
        std::vector<int> result;
        std::istringstream iss(msg);
        std::string token;

        while (std::getline(iss, token, '&')) {
            auto pos = token.find('=');
            if (pos != std::string::npos) {
                result.push_back(std::stoi(token.substr(pos + 1)));
            }
        }
        return result;
    }
    void handle_client(int client_socket, const std::string& client_ip, int client_port) {
        char buffer[4096];
        int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);

        if (bytes_received <= 0) {
            closesocket(client_socket);
            return;
        }

        std::string request(buffer, bytes_received);

        auto info = parse_http_request(request);

        if (info.command == "GET") {
            do_GET(client_socket, client_ip, client_port, info);
        }
        else if (info.command == "POST") {
            std::string delimiter = "\r\n\r\n";
            size_t pos = request.find(delimiter);
            std::string body = (pos != std::string::npos) ? request.substr(pos + delimiter.length()) : "";

            do_POST(client_socket, client_ip, client_port, info, body);
        }

        closesocket(client_socket);
    }
};


int main() {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    const int PORT = 8080;

    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        std::cerr << "Socket creation failed.\n";
        return 1;
    }

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_socket, (sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Bind failed.\n";
        return 1;
    }

    listen(server_socket, 5);
    std::cout << "## HTTP server started at http://localhost:" << PORT << "\n";

    MyHttpHandler handler;

    while (true) {
        sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);
        int client_socket = accept(server_socket, (sockaddr*)&client_addr, &client_len);
        if (client_socket < 0) continue;

        std::string client_ip = "127.0.0.1";
        int client_port = ntohs(client_addr.sin_port);

        std::thread(&MyHttpHandler::handle_client, &handler, client_socket, client_ip, client_port).detach();
    }

    closesocket(server_socket);
    WSACleanup();
    return 0;
}