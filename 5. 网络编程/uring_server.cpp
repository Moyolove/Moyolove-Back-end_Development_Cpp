#include<sys/socket.h>
#include<netinet/in.h>
#include<stdio.h>
#include<string.h>
#include<iostream>
#include<liburing.h>
#include<unistd.h>

#define ENTRIES_LENGTH 1024

enum {
    EVENT_ACCEPT = 0, 
    EVENT_READ,
    EVENT_WRITE
};

typedef struct _conninfo{
    int connfd;
    int event;
}conninfo;

void set_accept_event(struct io_uring *ring, int sockfd, struct sockaddr *addr,
                   socklen_t *addrlen, int flags){

    struct io_uring_sqe *sqe = io_uring_get_sqe(ring);

    io_uring_prep_accept(sqe, sockfd, addr, addrlen, flags);

    conninfo info_accept  = {
        .connfd = sockfd,
        .event = EVENT_ACCEPT,
    };
    memcpy(&sqe->user_data, &info_accept, sizeof(info_accept));
}

void set_recv_event(struct io_uring *ring, int sockfd, void *buf, size_t len, int flags){

    struct io_uring_sqe *sqe = io_uring_get_sqe(ring);

    io_uring_prep_recv(sqe, sockfd, buf, len, flags);
    conninfo info_read = {
        .connfd = sockfd,
        .event = EVENT_READ,
    };
    memcpy(&sqe->user_data, &info_read, sizeof(info_read));
}

void set_send_event(struct io_uring *ring, int sockfd, void *buf, size_t len, int flags){

    struct io_uring_sqe *sqe = io_uring_get_sqe(ring);

    io_uring_prep_send(sqe, sockfd, buf, len, flags);
    conninfo info_write = {
        .connfd = sockfd,
        .event = EVENT_WRITE,
    };
    memcpy(&sqe->user_data, &info_write, sizeof(info_write));
}

int main(){
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(9999);

    if(bind(listenfd, (struct sockaddr*)&servaddr, sizeof(struct sockaddr)) == -1){
        std::cout << "bind failed" << std::endl;
        return -1;
    }

    listen(listenfd, 10);

    io_uring_params paras;
    memset(&paras, 0, sizeof(paras));

    struct io_uring ring;
    io_uring_queue_init_params(ENTRIES_LENGTH, &ring, &paras);//这里会创建一个fd = 4
    io_uring_sqe *sqe = io_uring_get_sqe(&ring);

    struct sockaddr_in clientaddr;
    socklen_t clientlen = sizeof(clientaddr);
    
    set_accept_event(&ring, listenfd, (struct sockaddr*)&clientaddr, &clientlen, 0);

    char buffer[1024] = {0};

    while(1){
        io_uring_submit(&ring);

        struct io_uring_cqe *cqe;
        io_uring_wait_cqe(&ring, &cqe);

        struct io_uring_cqe *cqes[10];
        int cqecount = io_uring_peek_batch_cqe(&ring, cqes, 10);
        int i = 0;
        for(i = 0; i < cqecount; i++){
            cqe = cqes[i];

            conninfo ci;
            memcpy(&ci, &cqe->user_data, sizeof(ci));

            if(ci.event == EVENT_ACCEPT){
                int clientfd = cqe->res;

                if(clientfd < 0) continue;

                std::cout << "accept clientfd : " << clientfd << std::endl; //从5开始

                set_accept_event(&ring, ci.connfd, (struct sockaddr*)&clientaddr, &clientlen, 0);

                set_recv_event(&ring, clientfd, buffer, 1024, 0);

            }else if(ci.event == EVENT_READ){

                if(cqe->res > 0){
                    std::cout << "receive" << buffer << ", " << cqe->res << std::endl;

                    set_send_event(&ring, ci.connfd, buffer, cqe->res, 0);
                }
                if(cqe->res == 0) close(ci.connfd);
            }else if(ci.event == EVENT_WRITE){

                set_recv_event(&ring, ci.connfd, buffer, 1024, 0);
            }

            io_uring_cq_advance(&ring, cqecount);
        }

    }

    std::cin.get();
}