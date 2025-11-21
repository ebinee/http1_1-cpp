#include <iostream>
#include <string>
#include <sstream>
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")

std::string http_send_request(const std::string& host, int port, const std::string& request_msg) {
    WSADATA wsaData;
    int wsaResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (wsaResult != 0) {
        std::cerr << "WSAStartup failed: " << wsaResult << std::endl;
        return "";
    }

    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) return "";

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(host.c_str());

    if (connect(sock, (sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        closesocket(sock);
        WSACleanup();
        return "";
    }

    send(sock, request_msg.c_str(), request_msg.size(), 0);

    char buffer[4096];
    int received;
    std::stringstream response;

    while ((received = recv(sock, buffer, sizeof(buffer), 0)) > 0) {
        response.write(buffer, received);
    }

    closesocket(sock);
    WSACleanup();
    return response.str();
}

int main() {
    std::cout << "## HTTP client started." << std::endl;

    std::cout << "## GET request for http://localhost:8080/temp/" << std::endl;

    std::string get_msg1 =
        "GET /temp/ HTTP/1.1\r\n"
        "Host: localhost\r\n"
        "Connection: close\r\n\r\n";

    std::string response1 = http_send_request("127.0.0.1", 8080, get_msg1);

    std::cout << "## GET response [start]" << std::endl;
    std::cout << response1 << std::endl;
    std::cout << "## GET response [end]" << std::endl;

    std::cout << "## GET request for http://localhost:8080/?var1=9&var2=9" << std::endl;

    std::string get_msg2 =
        "GET /?var1=9&var2=9 HTTP/1.1\r\n"
        "Host: localhost\r\n"
        "Connection: close\r\n\r\n";

    std::string response2 = http_send_request("127.0.0.1", 8080, get_msg2);


    std::cout << response2 << std::endl;
    std::cout << "## GET response [end]" << std::endl;

    std::cout << "## POST request for http://localhost:8080/ with var1 is 9 and var2 is 9" << std::endl;

    std::string body = "var1=9&var2=9";

    std::ostringstream post_msg;
    post_msg <<
        "POST / HTTP/1.1\r\n"
        "Host: localhost\r\n"
        "Content-Type: application/x-www-form-urlencoded\r\n"
        "Content-Length: " << body.size() << "\r\n"
        "Connection: close\r\n\r\n"
        << body;

    std::string response3 = http_send_request("127.0.0.1", 8080, post_msg.str());

    std::cout << "## POST response [start]" << std::endl;
    std::cout << response3 << std::endl;
    std::cout << "## POST response [end]" << std::endl;


    std::cout << "## HTTP client completed." << std::endl;

    return 0;
}
