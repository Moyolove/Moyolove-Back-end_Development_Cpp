#include <iostream>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <thread>
#include <pthread.h>
#include <poll.h>

#define BUFFER_SIZE 1024
#define POLL_SIZE 1024

//1个connection 一个thread
void* client_thread(void *arg){
    int clientfd = *(int *)arg;
    while(1){
        char* buffer = new char[BUFFER_SIZE];
        int ret = recv(clientfd, buffer, BUFFER_SIZE, 0);
        if(ret == 0){
            close(clientfd);
            break;
        }
        std::cout << buffer << std::endl;
        send(clientfd, buffer, ret, 0);
    }
}

int main(){
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(struct sockaddr_in));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(9999);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); // 0.0.0.0

    if(bind(sockfd, (struct sockaddr*)&servaddr, sizeof(struct sockaddr)) == -1){
        std::cout << "bind failed : " << strerror(errno) << std::endl;
        return -1;
    }
    listen(sockfd, 10);

    struct sockaddr_in clientaddr;
    socklen_t len = sizeof(clientaddr);
    //获取io状态，并设置为非阻塞
    // int flags = fcntl(sockfd, F_GETFL, 0);
    // flags |= O_NONBLOCK;
    // fcntl(sockfd, F_SETFL, flags);

    

    //accept等待客户端连接，连接后阻塞解除
    //fd依次增加，标准输入输出以及错误分别占用0，1，2，sockfd为3，clientfd为4
    //即使在listen执行后，accept执行前有客户端连接，依然会被accept捕获
#if 0
    while(1){
        int clientfd = accept(sockfd, (struct sockaddr*)&clientaddr, &len);
        pthread_t thread_id;
        pthread_create(&thread_id, NULL, client_thread, &clientfd);
    }
#elif 0
    //select(maxnumOffd, rfds, wfds, efds, timeout);
    fd_set rfds, rset; //1024bit大小
    FD_SET(sockfd, &rfds);
    int maxfd = sockfd;
    int clientfd = 0;
    while(1){
        rset = rfds;
        int nready = select(maxfd + 1, &rset, NULL, NULL, NULL);
        //经过轮询socketfd门卫已就绪
        if(FD_ISSET(sockfd, &rset)){
            clientfd = accept(sockfd, (struct sockaddr*)&clientaddr, &len);
            std::cout << "accept :" << clientfd << std::endl;

            FD_SET(clientfd, &rfds);
            //断开的fd会被server回收
            if(clientfd > maxfd) maxfd = clientfd;
            if(--nready == 0) continue;
        }
        int i = 0;
        for(i = sockfd + 1; i <= maxfd; i++){
            if(FD_ISSET(i, &rset)){
                char* buffer = new char[BUFFER_SIZE];
                int ret = recv(i, buffer, BUFFER_SIZE, 0);
                if(ret == 0){
                    close(i);
                    break;
                }
                std::cout << buffer << std::endl;
                send(i, buffer, ret, 0);
            }
        }
    }
#else
    struct pollfd fds[POLL_SIZE] = {0};
    fds[sockfd].fd = sockfd;
    fds[sockfd].events = POLLIN;
    int maxfd = sockfd;
    int clientfd = 0;
    while(1){
        int nready = poll(fds, maxfd + 1, -1);
        if(fds[sockfd].revents & POLLIN){
            clientfd = accept(sockfd, (struct sockaddr*)&clientaddr, &len);
            std::cout << "accept :" << clientfd << std::endl;

            fds[clientfd].fd = clientfd;
            fds[clientfd].events = POLLIN;

            if(clientfd > maxfd) maxfd = clientfd;
            if(--nready == 0) continue;
        }
        int i = 0;
        for(int i = 0; i <= maxfd; i++){
            if(fds[i].revents & POLLIN){
                char* buffer = new char[BUFFER_SIZE];
                int ret = recv(i, buffer, BUFFER_SIZE, 0);
                if(ret == 0){
                    fds[i].fd = -1;
                    fds[i].events = 0;
                    close(i);
                    break;
                }
                std::cout << buffer << std::endl;
                send(i, buffer, ret, 0);
            }
        }
    }
#endif
    
    std::cin.get();
}