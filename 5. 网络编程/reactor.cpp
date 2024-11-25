#include<iostream>
#include<cstdlib>
#include<string.h>
#include<sys/socket.h>
#include<poll.h>
#include<sys/epoll.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<stdlib.h>

#define EVENTS_SIZE 1024
#define BUFFER_SIZE 1024
#define BLOCK_SIZE  1024

typedef int (*ZVCALLBACK)(int fd, int events, void* args);

typedef struct zv_connect_s{
    int fd;
    ZVCALLBACK cb;
    
    char* rbuffer = new char[BUFFER_SIZE];
    int rc;
    //根据协议确定下一次读多少
    int count;

    char* wbuffer = new char[BUFFER_SIZE];
    int wc;
}zv_connect_t;

typedef struct zv_connblock_s{
    zv_connect_t* block;
    struct zv_connblock_s* next;
}zv_connblock__t;

typedef struct zv_reactor_s{
    int epfd;
    //块的数量
    int blocknum;
    epoll_event events[EVENTS_SIZE];
    zv_connblock__t* zv_connblock__header;
}zv_reactor_t;

int zv_init_reactor(zv_reactor_t* reactor){

    if(!reactor) return -1;

    reactor->zv_connblock__header = new zv_connblock__t();
    if(!reactor->zv_connblock__header){
        return -1;
    }

    reactor->epfd = epoll_create(1);

    reactor->zv_connblock__header->block = new zv_connect_t[BLOCK_SIZE];
    reactor->blocknum = 1;
    reactor->zv_connblock__header->next = NULL;

    if(!reactor->zv_connblock__header->block){
        return -1;
    }
    return 0;
}

//从头部开始释放空间
void zv_dest_reactor(zv_reactor_t* reactor){
    if(!reactor) return;
    zv_connblock__t* cur = reactor->zv_connblock__header;
    zv_connblock__t* pre = reactor->zv_connblock__header;
    while(cur != NULL){
        pre = cur;
        cur = cur->next;
        
        if(pre->block) delete[] pre->block;

        if(pre) delete pre;
    }
    

    close(reactor->epfd);
}

//尾插法扩展空间
void zv_connect_openblock(zv_reactor_t* reactor){
    if(!reactor) return;
    zv_connblock__t* cur = reactor->zv_connblock__header;
    while(cur->next != NULL) cur = cur->next;

    cur->next = new zv_connblock__t();
    cur->next->next = NULL;
    cur->next->block = new zv_connect_t[BLOCK_SIZE];

    reactor->blocknum++;
}


zv_connect_t* zv_connect_idx(zv_reactor_t* reactor, int fd){
    if(!reactor) return NULL;

    int blockidx = fd / BLOCK_SIZE;

    
    //如果需要分配
    while(blockidx >= reactor->blocknum){
        zv_connect_openblock(reactor);        
    }

    int i = 0;
    zv_connblock__t* blockptr = reactor->zv_connblock__header;
    while(i < blockidx){
        i++;
        blockptr = blockptr->next;
    }

    return &blockptr->block[fd % BLOCK_SIZE];
}

int init_server(short port){
    int socketfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in sockaddr;
    memset(&sockaddr, 0, sizeof(sockaddr));
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = htons(port);
    sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(socketfd, (struct sockaddr*)&sockaddr, sizeof(sockaddr)) == -1){
        std::cout << "bind failed" << std::endl;
        return -1;
    }
    listen(socketfd, 10);

    return socketfd;
}

int recv_cb(int fd, int events, void* args);
int send_cb(int fd, int events, void* args){
    zv_reactor_t* reactor = (zv_reactor_t *)args;
    zv_connect_t* conn = zv_connect_idx(reactor, fd);
    
    //处理数据后存入wbuffer，此处仅以copy代替
    // memcpy(conn->wbuffer, conn->rbuffer, conn->rc);
    // conn->wc = conn->rc;

    send(fd, conn->wbuffer, conn->wc, 0);

    conn->cb = recv_cb;

    epoll_event ev;
    ev.data.fd = fd;
    ev.events = EPOLLIN;
    epoll_ctl(reactor->epfd, EPOLL_CTL_MOD, fd, &ev);

}


//适应协议的recv
int recv_cb(int fd, int events, void* args){

    zv_reactor_t* reactor = (zv_reactor_t *)args;
    zv_connect_t* conn = zv_connect_idx(reactor, fd);

    int ret = recv(fd, conn->rbuffer + conn->rc, conn->count, 0);

    if(ret < 0){
        return -1;
    }else if(ret == 0){
        conn->fd = -1;
        conn->rc = 0;
        conn->wc = 0;

        epoll_ctl(reactor->epfd, EPOLL_CTL_DEL, fd, NULL);

        close(fd);
        return 0;
    }

    std::cout << conn->rbuffer << std::endl;
    
    conn->rc += ret;
    //处理数据后存入wbuffer，此处仅以copy代替
    memcpy(conn->wbuffer, conn->rbuffer, conn->rc);
    conn->wc = conn->rc;
#if 0
    conn->count = *(short*)(conn->rbuffer);
    epoll_event ev;
    ev.data.fd = fd;
    ev.events = EPOLLIN;
    epoll_ctl(reactor->epfd, EPOLL_CTL_ADD, fd, &ev);
#else
    conn->cb = send_cb;

    epoll_event ev;
    ev.data.fd = fd;
    ev.events = EPOLLOUT;
    epoll_ctl(reactor->epfd, EPOLL_CTL_MOD, fd, &ev);
#endif
    return 0;
}

int accept_cb(int fd, int events, void* args){
    struct sockaddr_in clientaddr;
    socklen_t len = sizeof(clientaddr);
    int clientfd = 0;

    clientfd = accept(fd, (struct sockaddr*)&clientaddr, &len);

    if(clientfd < 0) return -1;

    zv_reactor_t* reactor = (zv_reactor_t*) args;

    zv_connect_t* conn = zv_connect_idx(reactor, clientfd);
    conn->fd = clientfd;
    conn->cb = recv_cb;
    //假设此次所用协议要求先读2个字节
    conn->count = 2;
    
    std::cout << "accept : " << clientfd << std::endl;

    epoll_event ev;
    ev.data.fd = clientfd;
    ev.events = EPOLLIN;
    epoll_ctl(reactor->epfd, EPOLL_CTL_ADD, clientfd, &ev);
    return 0;
}



int set_listener(int fd, zv_reactor_t* reactor, ZVCALLBACK cb){
    
    if(!reactor || !reactor->zv_connblock__header) return -1;

    reactor->zv_connblock__header->block[fd].fd = fd;
    reactor->zv_connblock__header->block[fd].cb = cb;
    

    epoll_event ev;
    ev.data.fd = fd;
    ev.events = EPOLLIN;

    epoll_ctl(reactor->epfd, EPOLL_CTL_ADD, fd, &ev);

    return 0;
}

int main(int argc, char* argv[]){
    
    if(argc < 2) return -1;
    short port = atoi(argv[1]);
    int socketfd = init_server(port);


    zv_reactor_t reactor;

    zv_init_reactor(&reactor);

    set_listener(socketfd, &reactor, accept_cb);//当listened fd有事件时，触发回调accept_cb


    epoll_event events[EVENTS_SIZE] = {0};

    while(1){
        //wait会将就绪的io拷贝到events中，而不像poll那样全部拷贝
        //mainloop。由事件驱动对应的回调函数
        int nready = epoll_wait(reactor.epfd, events, EVENTS_SIZE, -1);
        if(nready < 0) continue;
        int i = 0;
        for(i = 0; i < nready; i++){
            int confd = events[i].data.fd;
            zv_connect_t* conn = zv_connect_idx(&reactor, confd);
            
            if(events[i].events & EPOLLIN){
                conn->cb(confd, events[i].events, &reactor);
            }

            if(events[i].events & EPOLLOUT){
                conn->cb(confd, events[i].events, &reactor);
            }
        }
    }
}