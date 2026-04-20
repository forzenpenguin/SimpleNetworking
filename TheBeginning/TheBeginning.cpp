#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <cstdlib>
#include <thread>
#include <cstring>
#pragma comment(lib, "Ws2_32.lib")

#define BACKLOG 10
#define PORT "3490"
#define NODE "172.29.66.224"

using namespace std;

void checker() {
    WSAData wsadata;
    if (WSAStartup(MAKEWORD(2, 2), &wsadata) != 0) {
        cout << "WSAStartup failed." << endl;
    }
    else {
        cout << "WSAStartup works fine." << endl;
        WSACleanup(); // clean up after success
    }
}
void* get_addr(struct sockaddr* sa) {
    if (sa->sa_family == AF_INET)
        return &((struct sockaddr_in*)sa)->sin_addr;
    if (sa->sa_family == AF_INET6)
        return &((struct sockaddr_in6*)sa)->sin6_addr;
}
void server() {

    int status;
    int yes = 1;
    struct addrinfo hints, *res, *p;
    SOCKET sockfd;
	struct sockaddr_storage their_addr;
	int sin_size = 0;
	char s[INET6_ADDRSTRLEN];

    memset(&hints, 0, sizeof hints);
	hints.ai_flags = AI_PASSIVE;
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
    status = getaddrinfo(NULL, PORT, &hints, &res);

    if (status == -1)
        cout << "adderinfo error" << endl;
    for (p= res; p != NULL;  p= p->ai_next) {
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sockfd == -1)
			cerr << "socket error" << endl;
            continue;
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const char *)&yes, sizeof(int)) == -1)
            cerr << "setsockopt error" << endl;
            exit(1);
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			cerr << "bind error" << endl;
            closesocket(sockfd);
            continue;
        }
        break; // Successfully bound
    }
    freeaddrinfo(res);
    if (res == NULL) {
        cerr << "Failed to bind to any address." << endl;
        exit(1);
    }

    if(listen(sockfd, BACKLOG) == -1) {
        cerr << "listen error" << endl;
        exit(1);
	}

    while (1) {
        sin_size = sizeof their_addr;
        SOCKET new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if(new_fd == -1)
			cerr << "accept error" << endl;
        inet_ntop(their_addr.ss_family, get_addr((struct sockaddr *)&their_addr), s, sizeof s);
        cout << "We got connection from: " << s << endl;

        // Windows does not support fork(); use a detached thread to handle the client.
        {
            std::thread([new_fd]() {
                const char* msg = "Hello World!";
                int len = (int)strlen(msg);
                if (send(new_fd, msg, len, 0) == SOCKET_ERROR)
                    cerr << "send error: " << WSAGetLastError() << endl;
                closesocket(new_fd);
            }).detach();
        }
    }
}

void client_func() {
    SOCKET sockfd;
	char buffer[1024];
    struct addrinfo hints, * res, * p;
    int result, numbytes;
    char s[INET6_ADDRSTRLEN];
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    result = getaddrinfo(NULL, PORT, &hints, &res);
    if(result == -1)
		cerr << "getaddrinfo error" << endl;
    for (p = res; p != NULL; p = p->ai_next) {
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sockfd == -1)
            continue;
        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1)
            continue;
        break;
    }
    if (p == NULL)
        cout << "connect failed";
    inet_ntop(p->ai_family, get_addr((struct sockaddr *)p->ai_addr), s, sizeof s);
    cout << "Connected to: " << s << endl;
	freeaddrinfo(res);
    FD_SET readfds;
	FD_ZERO(&readfds);
	FD_SET(sockfd, &readfds);

	struct timeval tv;
	tv.tv_sec = 3;          // 3 seconds
	tv.tv_usec = 500000;    // 500 milliseconds -> total 3500 ms

	int rv = select(0, &readfds, NULL, NULL, &tv);
	if (rv == SOCKET_ERROR) {
		cerr << "select error: " << WSAGetLastError() << endl;
	} else if (rv == 0) {
		cerr << "select timeout" << endl;
	} else {
		if (FD_ISSET(sockfd, &readfds)) {
			int numbytes = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
			if (numbytes == SOCKET_ERROR) {
				cerr << "recv error: " << WSAGetLastError() << endl;
			} else {
				buffer[numbytes] = '\0';
				cout << "Received: " << buffer << endl;
			}
		}
	}
    closesocket(sockfd);
}



void* get_addrinf0(struct sockaddr* sa) {
    if (sa->sa_family == AF_INET)
        return &((struct sockaddr_in*)sa)->sin_addr;
    if (sa->sa_family == AF_INET6)
        return &((struct sockaddr_in6*)sa)->sin6_addr;
    return nullptr;
};
char get_port(struct sockaddr* sa) {
    switch (sa->sa_family) {
    case AF_INET:
        return ((struct sockaddr_in*)sa)->sin_port;
        break;
    case AF_INET6:
        return ((struct sockaddr_in6*)sa)->sin6_port;
        break;
    }
};


int main() {
    return 0;
}