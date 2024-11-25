#include<iostream>
#include<cstdlib>
#include<string.h>
#include<sys/socket.h>
#include<poll.h>
#include<sys/epoll.h>
#include<arpa/inet.h>
#include<unistd.h>

#define EVENTS_SIZE 1024
#define BUFFER_SIZE 1024

int main(){
    int socketfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in sockaddr;
    memset(&sockaddr, 0, sizeof(sockaddr));
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = htons(9999);
    sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(socketfd, (struct sockaddr*)&sockaddr, sizeof(sockaddr)) == -1){
        std::cout << "bind failed" << std::endl;
        return -1;
    }
    listen(socketfd, 10);

    struct sockaddr_in clientaddr;
    socklen_t len = sizeof(clientaddr);
    int clientfd = 0;

    int epfd = epoll_create(1);

    epoll_event ev;
    ev.data.fd = socketfd;
    ev.events = EPOLLIN;

    epoll_ctl(epfd, EPOLL_CTL_ADD, socketfd, &ev);

    epoll_event events[EVENTS_SIZE] = {0};

    while(1){
        //wait会将就绪的io拷贝到events中，而不像poll那样全部拷贝
        int nready = epoll_wait(epfd, events, EVENTS_SIZE, -1);
        if(nready < 0) continue;
        int i = 0;
        for(i = 0; i < nready; i++){
            int confd = events[i].data.fd;
            if(confd == socketfd){
                clientfd = accept(socketfd, (struct sockaddr*)&clientaddr, &len);
                if(clientfd <= 0){
                    continue;
                }
                std::cout << "accept : " << clientfd << std::endl;

                ev.events = EPOLLIN;
                ev.data.fd = clientfd;
                //将clientfd加入到all set中
                epoll_ctl(epfd, EPOLL_CTL_ADD, clientfd, &ev);
            }else if(events[i].events & EPOLLIN){
                char* buffer = new char[BUFFER_SIZE];
                int ret = recv(confd, buffer, BUFFER_SIZE, 0);
                if(ret == 0){
                    //epoll不会在断开后将fd自动删除
                    epoll_ctl(epfd, EPOLL_CTL_DEL, confd, NULL);
                    close(confd);
                    std::cout << "clientfd : " << confd << " closed" << std::endl;
                }
                std::cout << buffer << std::endl;
            }
        }
    }
}