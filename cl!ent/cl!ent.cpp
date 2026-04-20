// client.cpp : This file contains the 'main' function. Program execution begins and ends there.
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

void SimpleClient() {
    cout << "CLIENT" << endl;
    struct addrinfo hints, * res, * p;
    int result;
    SOCKET sockfd;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    cout << "setting" << endl;
    result = getaddrinfo(NODE, PORT, &hints, &res);
    cout << "getting" << endl;
    if (result != 0)
        cerr << "getaddrinfo error: " << WSAGetLastError() << endl;
    cout << "connecting" << endl;
    for (p = res; p != NULL; p = p->ai_next) {
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sockfd == INVALID_SOCKET)
            cerr << "socket error: " << WSAGetLastError() << endl;
        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == SOCKET_ERROR)
            cerr << "connect error: " << WSAGetLastError() << endl;
        break;
    }
    if (p == NULL)
        cerr << "Failed to connect to server." << endl;
    freeaddrinfo(res);
    char buf[1224];
    int len;
    cout << "reeiving" << endl;
    int recievLen = recv(sockfd, buf, 1223, 0);
    if (recievLen == SOCKET_ERROR)
        cerr << "receive error: " << WSAGetLastError() << endl;
    buf[recievLen] = '\0';
    cout << "end" << endl;
    cout << "Received: " << buf << endl;

}
int main()
{
    checker();
    SimpleClient();
    WSACleanup(); // clean up after we're done with Winsock
}