// client.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <cstdlib>
#include <thread>
#include <cstring>
#include <string>
#pragma comment(lib, "Ws2_32.lib")

#define BACKLOG 10
using namespace std;

class client {
protected:
    struct addrinfo hints, * res, * p;
    int result;
    int sockfd;
    fd_set readfd;
    fd_set writefd;
    char buf[1024];
    char s[INET6_ADDRSTRLEN];
    string msg = "";

private:
    // member functions
    void checker() {
        WSAData wsadata;
        if (WSAStartup(MAKEWORD(2, 2), &wsadata) != 0) {
            cout << "WSAStartup failed." << endl;
        }
        else {
            cout << "WSAStartup works fine." << endl;
        }
    }
    void* get_addrinf0(struct sockaddr* sa) {
        if (sa->sa_family == AF_INET)
            return &((struct sockaddr_in*)sa)->sin_addr;
        if (sa->sa_family == AF_INET6)
            return &((struct sockaddr_in6*)sa)->sin6_addr;
        return nullptr;
    };
public:
    // constructor and destructor
    client(const char* ip_param, const char* port_param) {
        checker();
        cout << "CLIENT" << endl;
        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        result = getaddrinfo(ip_param, port_param, &hints, &res);
        if (result != 0) {
            cerr << "getaddrinfo error: " << WSAGetLastError() << endl;
            return;
        }
        for (p = res; p != NULL; p = p->ai_next) {
            sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
            if (sockfd == INVALID_SOCKET) {
                cerr << "socket error: " << WSAGetLastError() << endl;
                continue;
            }
            if (connect(sockfd, p->ai_addr, p->ai_addrlen) == SOCKET_ERROR) {
                cerr << "connect error: " << WSAGetLastError() << endl;
                closesocket(sockfd);
                continue;
            }
            break;
        }
        if (p == NULL) {
            cerr << "Failed to connect to server: " << WSAGetLastError() << endl;
            return;
        }
        freeaddrinfo(res);
    };
    ~client() {
        closesocket(sockfd);
        WSACleanup();
    }
    // member functions continues
private:
    void sendMessage() {
        int len = (int)msg.size(), bytes_sent;
        if ((bytes_sent = send(sockfd, msg.c_str(), len, 0)) == SOCKET_ERROR)
            cerr << "send error: " << WSAGetLastError() << endl;
        msg = "";
    }
public:
    void receiveMessage() {
        fd_set readfd;
        fd_set writefd;
        FD_ZERO(&readfd);
        FD_ZERO(&writefd);
        FD_SET(sockfd, &readfd);
        FD_SET(sockfd, &writefd);
        int rv = select(sockfd + 1, &readfd, &writefd, NULL, NULL); // I hardcoded the first parameter to be sockfd + 1 because it's the only socket we're monitoring, but in a more complex client with multiple sockets, this should be set to the highest socket descriptor plus one.
        if (rv == SOCKET_ERROR) {
            cout << "select error: " << WSAGetLastError() << endl;
            return;
        }

        if (FD_ISSET(sockfd, &readfd)) {
            int recievLen = recv(sockfd, buf, 1023, 0);
            if (recievLen == SOCKET_ERROR)
                cerr << "receive error: " << WSAGetLastError() << endl;
            buf[recievLen] = '\0';
            inet_ntop(p->ai_family, get_addrinf0((struct sockaddr*)p->ai_addr), s, sizeof s);
            cout << s << ": " << buf << endl;
            buf[0] = '\0';
            cout << "-----------------------------" << endl;
            cout << "You: ";
            getline(cin, msg);
        };
        if (msg != "" && FD_ISSET(sockfd, &writefd)) {
            sendMessage();
        }
    }
};


int main()
{
    client myClient("10.56.57.48", "1080");
    while (true) {
        myClient.receiveMessage();
    }
}