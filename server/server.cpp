// networking.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <cstdlib>
#include <thread>
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
    while (1) {
        sin_size = sizeof their_addr;
        new_sockfd = accept(sockfd, (struct  sockaddr*)&their_addr, &sin_size);
        if (new_sockfd == INVALID_SOCKET) {
            cout << "accept error: " << WSAGetLastError() << endl;
            return;
        }
		FD_SET(new_sockfd, &readfds);
        int n = new_sockfd + 1;
        int rv = select(n, &readfds, NULL, NULL, NULL);
        inet_ntop(their_addr.ss_family, get_addrinf0((struct sockaddr*)&their_addr), s, sizeof s);
        if (rv == SOCKET_ERROR) {
            cout << "select error: " << WSAGetLastError() << endl;
            return;
        }
        for (int i = 0; i < n; i++) {
            if (FD_ISSET(i, &readfds)) {
				int bytes_received = recv(i, buf, sizeof buf, 0);
                buf[bytes_received] = '\0';
				cout << s << ": " << string(buf, bytes_received) << endl;
                cout << "==============================================" << endl;
				buf[0] = '\0';
            }
		}

        //const char* msg = "Hello World!";
        //int len = (int)strlen(msg), bytes_sent;
        //if ((bytes_sent = send(new_sockfd, msg, len, 0)) == SOCKET_ERROR)
        //    cerr << "send error: " << WSAGetLastError() << endl;
        closesocket(new_sockfd);
    };
};


int main()
{
    checker();
    SimpleServer();
    WSACleanup(); // clean up after we're done with Winsock
}

