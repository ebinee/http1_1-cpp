#include <iostream>
#include <string>
#include <winsock2.h>
#include "json.hpp"
#pragma comment(lib, "ws2_32.lib")

using json = nlohmann::json;

std::string http_request(const std::string& method,
    const std::string& url_path,
    const std::string& body = "")
{
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);

    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in server{};
    server.sin_family = AF_INET;
    server.sin_port = htons(8080);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    connect(sock, (sockaddr*)&server, sizeof(server));

    std::string request;
    request += method + " " + url_path + " HTTP/1.1\r\n";
    request += "Host: 127.0.0.1\r\n";

    if (method == "POST" || method == "PUT") {
        request += "Content-Type: application/x-www-form-urlencoded\r\n";
        request += "Content-Length: " + std::to_string(body.size()) + "\r\n";
    }

    request += "Connection: close\r\n\r\n";

    if (body.size() > 0)
        request += body;

    send(sock, request.c_str(), request.size(), 0);

    char buffer[4096];
    int received;
    std::string response;

    while ((received = recv(sock, buffer, 4096, 0)) > 0) {
        response.append(buffer, received);
    }

    closesocket(sock);
    WSACleanup();
    return response;
}

json get_json_from_response(const std::string& resp) {
    int pos = resp.find("{");
    if (pos == std::string::npos) return json{};
    std::string js = resp.substr(pos);
    return json::parse(js);
}

int main() {
    std::cout << "## REST Client Started\n";

    auto r1 = http_request("GET", "/membership_api/0001");
    auto j1 = get_json_from_response(r1);
    std::cout << "#1 JSON: " << j1.dump() << "\n";

    auto r2 = http_request("POST", "/membership_api/0001", "0001=apple");
    auto j2 = get_json_from_response(r2);
    std::cout << "#2 JSON: " << j2.dump() << "\n";

    auto r3 = http_request("GET", "/membership_api/0001");
    auto j3 = get_json_from_response(r3);
    std::cout << "#3 JSON: " << j3.dump() << "\n";

    auto r4 = http_request("POST", "/membership_api/0001", "0001=xpple");
    auto j4 = get_json_from_response(r4);
    std::cout << "#4 JSON: " << j4.dump() << "\n";

    auto r5 = http_request("PUT", "/membership_api/0002", "0002=xrange");
    auto j5 = get_json_from_response(r5);
    std::cout << "#5 JSON: " << j5.dump() << "\n";

    http_request("POST", "/membership_api/0002", "0002=xrange");
    auto r6 = http_request("PUT", "/membership_api/0002", "0002=orange");
    auto j6 = get_json_from_response(r6);
    std::cout << "#6 JSON: " << j6.dump() << "\n";

    auto r7 = http_request("DELETE", "/membership_api/0001");
    auto j7 = get_json_from_response(r7);
    std::cout << "#7 JSON: " << j7.dump() << "\n";

    auto r8 = http_request("DELETE", "/membership_api/0001");
    auto j8 = get_json_from_response(r8);
    std::cout << "#8 JSON: " << j8.dump() << "\n";

    std::cout << "## REST Client Done\n";
    return 0;
}