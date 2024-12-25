#include<iostream>

#define _GNU_SOURCE
#include <dlfcn.h>

#include<sys/socket.h>
#include<sys/types.h>

typedef int (*connect_t)(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
connect_t connect_f = NULL;

typedef ssize_t (*recv_t)(int sockfd, void *buf, size_t len, int flags);
recv_t recv_f = NULL;

typedef ssize_t (*send_t)(int sockfd, const void *buf, size_t len, int flags);
send_t send_f = NULL;

int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen){
    std::cout << "connect" << std::endl;
    return connect_f(sockfd, addr, addrlen);
}

ssize_t recv(int sockfd, void *buf, size_t len, int flags){
    std::cout << "recv" << std::endl;
    return recv_f(sockfd, buf, len, flags);
}


ssize_t send(int sockfd, void *buf, size_t len, int flags){
    std::cout << "send" << std::endl;
    return send_f(sockfd, buf, len, flags);
}


void init_hook(){
    if( !connect_f )
        connect_f = (connect_t)(RTLD_NEXT, "connect");
    if( !recv_f )
        recv_f = (recv_t)dlsym(RTLD_NEXT, "recv");
    if( !send_f )
        send_f = (send_t)dlsym(RTLD_NEXT, "send");
}


int main(){


    return 0;
}