#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <cstdlib>
#include <thread>
#include <string>
#include <cstring>
#include <vector>
#pragma comment(lib, "Ws2_32.lib")

#define BACKLOG 10
#define PORT "1080"
#define NODE "10.56.57.48"
using namespace std;

class server {
protected:
	SOCKET server_socket;
	vector<SOCKET> clients;
    struct addrinfo hints, * res, * p;
    struct sockaddr_storage their_addr;
    int sin_size;
    int result;
    int listening;
    char s[INET6_ADDRSTRLEN];
    char buf[1024];

public:
    void checker() {
        WSAData wsaData;
        int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (result != 0) {
            cout << "WSAStartup failed." << result << endl;
        }
        else {
            cout << "WSAStartup works fine." << endl;
        }
    }
    server(const char* ip_param, const char* port_param) {
		checker();

        const char yes = '1';
        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_UNSPEC;
        hints.ai_flags = AI_PASSIVE;
        hints.ai_socktype = SOCK_STREAM;

        if ((result = getaddrinfo(ip_param, port_param, &hints, &res)) != 0) {
            cout << "getaddrinfo error: " << WSAGetLastError() << endl;
        }
        for (p = res; p != NULL; p = p->ai_next) {
            if ((server_socket = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == INVALID_SOCKET) {
                cout << "socket error: " << WSAGetLastError() << endl;
                continue;
            }
            if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(char)) == SOCKET_ERROR)
                continue;
            if (bind(server_socket, p->ai_addr, p->ai_addrlen) == SOCKET_ERROR)
                continue;
            break;
        }
        if (p == NULL) {
            cout << "An error happen while setting up the socket and binding it: " << WSAGetLastError() << endl;
            return;
        }

        freeaddrinfo(res);
        if ((listening = listen(server_socket, BACKLOG)) == SOCKET_ERROR) {
            cout << "listen error: " << WSAGetLastError() << endl;
        }
		cout << "Waiting for connections..." << endl;
	};
    ~server() {
        closesocket(server_socket);
        for(SOCKET& s: clients)
			closesocket(s);
        cout << "closed all sockets" << endl;
        WSACleanup();
	};
	// This function gets the address information from a sockaddr structure, depending on the address family (IPv4 or IPv6).
    void* get_addrinf0(struct sockaddr* sa) {
        if (sa->sa_family == AF_INET)
            return &((struct sockaddr_in*)sa)->sin_addr;
        if (sa->sa_family == AF_INET6)
            return &((struct sockaddr_in6*)sa)->sin6_addr;
        return nullptr;
    };
	// This function accepts incoming connections and adds them to the clients vector.
    void acceptConnections() {
            sin_size = sizeof their_addr;
            SOCKET new_sockfd = accept(server_socket, (struct  sockaddr*)&their_addr, &sin_size);
            if (new_sockfd == INVALID_SOCKET) {
                cout << "accept error: " << WSAGetLastError() << endl;
                return;
            }
            inet_ntop(their_addr.ss_family, get_addrinf0((struct sockaddr*)&their_addr), s, sizeof s);
            cout << "New connection from: " << s << endl;
            clients.push_back(new_sockfd);
    }
    void broadcastMessage(const char* message, SOCKET sender) {
        for (SOCKET& client : clients) {
            int len = (int)strlen(message), bytes_sent;
            if (client == sender)
                continue;
            if ((bytes_sent = send(client, message, len, 0)) == SOCKET_ERROR)
                cerr << "send error: " << WSAGetLastError() << endl;
        }
		buf[0] = '\0';
	}
    void receiveMessages() {
        fd_set readfds;
        while (true) {
            FD_ZERO(&readfds);
            for (SOCKET& client : clients) {
                FD_SET(client, &readfds);
            }
            int n = clients.back() + 1;
            int rv = select(n, &readfds, NULL, NULL, NULL);
            if (rv == SOCKET_ERROR) {
                cout << "select error: " << WSAGetLastError() << endl;
                return;
            }
            for (SOCKET& client : clients) {
                if (FD_ISSET(client, &readfds)) {
                    int bytes_received = recv(client, buf, sizeof buf, 0);
                    if (bytes_received > 0) {
                        buf[bytes_received] = '\0';
                        cout << "Message from client: " << buf << endl;
                        broadcastMessage(buf,client);
                    } else if (bytes_received == 0) {
                        cout << "Client disconnected." << endl;
                        closesocket(client);
                        clients.erase(remove(clients.begin(), clients.end(), client), clients.end());
                    } else {
                        cout << "recv error: " << WSAGetLastError() << endl;
                        clients.erase(remove(clients.begin(), clients.end(), client), clients.end());
                        return;

                    }
                }
            }
        }
	}
};

int main()
{
    server myServer("10.56.57.48", "1080");
    while (true) {
        myServer.acceptConnections();
        thread receiveThread([&myServer]() {myServer.receiveMessages(); });
        receiveThread.detach();
    }
}

