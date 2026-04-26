// networking.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <cstdlib>
#include <thread>
#include <string>
#include <cstring>
#pragma comment(lib, "Ws2_32.lib")

#define BACKLOG 10
#define PORT "1080"
#define NODE "10.64.115.48"
using namespace std;

// This function checks if WSAStartup works fine and prints the result.
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

// This function gets the address information from a sockaddr structure, depending on the address family (IPv4 or IPv6).
void* get_addrinf0(struct sockaddr* sa) {
    if (sa->sa_family == AF_INET)
        return &((struct sockaddr_in*)sa)->sin_addr;
    if (sa->sa_family == AF_INET6)
        return &((struct sockaddr_in6*)sa)->sin6_addr;
    return nullptr;
};


void SimpleServer() {
    cout << "SERVER" << endl;
    const char yes = '1';
    SOCKET sockfd;
    int new_sockfd;
    struct addrinfo hints, * res, * p;
    struct sockaddr_storage their_addr;
    int sin_size;
    int result;
    int listening;
    char s[INET6_ADDRSTRLEN];
    fd_set readfds;
	fd_set writefds;
	char buf[1024];
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_socktype = SOCK_STREAM;
    if ((result = getaddrinfo(NODE, PORT, &hints, &res)) != 0) {
        cout << "getaddrinfo error: " << WSAGetLastError() << endl;
    }
    for (p = res; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == INVALID_SOCKET) {
            cout << "socket error: " << WSAGetLastError() << endl;
            continue;
        }
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(char)) == SOCKET_ERROR)
            continue;
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == SOCKET_ERROR)
            continue;
        break;
    }
    if (p == NULL) {
        cout << "An error happen while setting up the socket and binding it: " << WSAGetLastError() << endl;
        return;
    }

    freeaddrinfo(res);
    if ((listening = listen(sockfd, BACKLOG)) == SOCKET_ERROR) {
        cout << "listen error: " << WSAGetLastError() << endl;
		return;
    };
    cout << "Waiting for connections..." << endl;
    FD_ZERO(&readfds);
	FD_ZERO(&writefds);
    while (1) {
        sin_size = sizeof their_addr;
        new_sockfd = accept(sockfd, (struct  sockaddr*)&their_addr, &sin_size);
        if (new_sockfd == INVALID_SOCKET) {
            cout << "accept error: " << WSAGetLastError() << endl;
            return;
        }
        inet_ntop(their_addr.ss_family, get_addrinf0((struct sockaddr*)&their_addr), s, sizeof s);
		cout << "Got connection from " << s << endl;
        int bytes_received = recv(new_sockfd, buf, sizeof buf, 0);
        buf[bytes_received] = '\0';
        cout << s << ": " << buf << endl;
		FD_SET(new_sockfd, &readfds);
        FD_SET(new_sockfd, &writefds);
        int n = new_sockfd + 1;
        int rv = select(n, &readfds, &writefds, NULL, NULL);
        if (rv == SOCKET_ERROR) {
            cout << "select error: " << WSAGetLastError() << endl;
            return;
        }
        string msg = "";
        while (true) {
            if (FD_ISSET(new_sockfd, &readfds)) {
                int bytes_received = recv(new_sockfd, buf, sizeof buf, 0);
                buf[bytes_received] = '\0';
                cout << s << ": " << buf << endl;
                cout << "==============================================" << endl;
                if (buf[0] != '\0') {
                    cout << "Typing: ";
                    getline(cin, msg);
                }
                buf[0] = '\0';
            }
            if (!msg.empty() && FD_ISSET(new_sockfd, &writefds)) {
                int len = (int)msg.size(), bytes_sent;
                if ((bytes_sent = send(new_sockfd, msg.c_str(), len, 0)) == SOCKET_ERROR)
                    cerr << "send error: " << WSAGetLastError() << endl;
                msg = "";
            }
        }
        closesocket(new_sockfd);
    }
};


int main()
{
    checker();
    SimpleServer();
    WSACleanup(); // clean up after we're done with Winsock
}

