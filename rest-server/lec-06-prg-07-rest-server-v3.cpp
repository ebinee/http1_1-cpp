#include <iostream>
#include <string>
#include <unordered_map>
#include <sstream>
#include <winsock2.h>
#include "json.hpp"
#pragma comment(lib, "ws2_32.lib")

using json = nlohmann::json;

class MembershipHandler {
public:
    std::unordered_map<std::string, std::string> database;

    json create(const std::string& id, const std::string& value) {
        if (database.count(id))
            return json{ {id, "None"} };
        else {
            database[id] = value;
            return json{ {id, database[id]} };
        }
    }

    json read(const std::string& id) {
        if (database.count(id))
            return json{ {id, database[id]} };
        else
            return json{ {id, "None"} };
    }

    json update(const std::string& id, const std::string& value) {
        if (database.count(id)) {
            database[id] = value;
        return json{ {id, database[id]} };
        }
        else
            return json{ {id, "None"} };
    }

    json remove(const std::string& id) {
        if (database.count(id)) {
            database.erase(id);
            return json{ {id, "Removed"} };
        }
        else
            return json{ {id, "None"} };
    }
};

MembershipHandler myManager;

class MembershipManager {
public:
    void handle_request(SOCKET client, const std::string& req) {
        std::string method;
        int sp = req.find(' ');
        if (sp != -1)
            method = req.substr(0, sp);

        std::string path;

        int sp2 = req.find(' ', sp + 1);
        if (sp2 != -1)
            path = req.substr(sp + 1, sp2 - sp - 1);

        std::string prefix = "/membership_api/";
        if (path.find(prefix) != 0) {
            send_text(client, "wrong url");
            return;
        }

        std::string member_id = path.substr(prefix.size());
        std::string body = "";
        int body_pos = req.find("\r\n\r\n");
        if (body_pos != -1) {
            body = req.substr(body_pos + 4);
        }

        std::string value = "";
        int eq = body.find('=');
        if (eq != -1) {
            value = body.substr(eq + 1);
        }

        json response;

        if (method == "POST")
            response = myManager.create(member_id, value);
        else if (method == "GET")
            response = myManager.read(member_id);
        else if (method == "PUT")
            response = myManager.update(member_id, value);
        else if (method == "DELETE")
            response = myManager.remove(member_id);
        else {
            send_text(client, "bad method");
            return;
        }

        send_json(client, response.dump());
    }

    void send_text(SOCKET client, const std::string& msg) {
        std::string header =
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/plain\r\n"
            "Connection: close\r\n\r\n";

        send(client, header.c_str(), header.size(), 0);
        send(client, msg.c_str(), msg.size(), 0);
    }

    void send_json(SOCKET client, const std::string& json_str) {
        std::ostringstream oss;
        oss << "HTTP/1.1 200 OK\r\n";
        oss << "Content-Type: application/json\r\n";
        oss << "Connection: close\r\n\r\n";
        oss << json_str;

        auto res = oss.str();
        send(client, res.c_str(), res.size(), 0);
    }
};

int main() {
    WSADATA wsa;
    int result = WSAStartup(MAKEWORD(2, 2), &wsa);

    SOCKET server = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(8080);

    bind(server, (sockaddr*)&addr, sizeof(addr));
    listen(server, 5);

    std::cout << "## REST server started at http://localhost:8080\n";

    MembershipManager mem;

    while (true) {
        sockaddr_in client_addr{};
        int len = sizeof(client_addr);
        SOCKET client = accept(server, (sockaddr*)&client_addr, &len);
        if (client == INVALID_SOCKET) continue;

        char buffer[4096];
        int received = recv(client, buffer, 4096, 0);
        if (received > 0) {
            std::string req(buffer, received);
            mem.handle_request(client, req);
        }

        closesocket(client);
    }

    closesocket(server);
    WSACleanup();
}