#include<iostream>
#include<cstdlib>
#include<string.h>
#include<sys/socket.h>
#include<poll.h>
#include<sys/epoll.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<stdlib.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<sys/sendfile.h>


#define EVENTS_SIZE 1024
#define BUFFER_SIZE 1024
#define BLOCK_SIZE  1024
#define KEY_MAX_LENGTH 128
#define VALUE_MAX_LENGTH 512
#define MAX_KEY_COUNT 128

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

    char* resource = new char[BUFFER_SIZE];

    bool enable_sendfile = 0;

    struct zv_kvstore_s* header;

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

typedef struct zv_kvpair_s{
    //提前分配减少内存碎片
    char key[KEY_MAX_LENGTH];
    char value[VALUE_MAX_LENGTH];

}zv_kvpair_t;

typedef struct zv_kvstore_s{
    zv_kvpair_t* table;
    int maxnum;
    int count;
}zv_kvstore_t;

int init_kvstore(zv_kvstore_t* kvstore){
    if(!kvstore) return -1;
    kvstore->table = new zv_kvpair_t[MAX_KEY_COUNT];
    if(!kvstore->table) return -2;
    memset(kvstore->table, 0, MAX_KEY_COUNT * sizeof(zv_kvpair_t));
    kvstore->maxnum = MAX_KEY_COUNT;
    kvstore->count = 0;

    return 0;
}

void dest_kvstore(zv_kvstore_t* kvstore){
    if(!kvstore) return;
    if(!kvstore->table){
        delete[] kvstore->table;
    }
}


int put_kvpair(zv_kvstore_t* kvstore, const char* key, const char* value){
    if(!kvstore || !kvstore->table || !key || !value) return -1;

    strncpy(kvstore->table[kvstore->count].key, key, KEY_MAX_LENGTH);
    strncpy(kvstore->table[kvstore->count].value, value, VALUE_MAX_LENGTH);
    kvstore->count++;

    return 0;

}

char* get_kvpair(zv_kvstore_t* kvstore, const char* key){
    if(!kvstore || !kvstore->table || !key) return NULL;

    for(int i = 0; i < kvstore->count; i++){
        if(*(kvstore->table[i].key) == *key) return kvstore->table[i].value;
    }

    return NULL;
}

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
// http request
// GET / HTTP/1.1
// Host: 192.168.229.129:9999
// Connection: keep-alive
// Upgrade-Insecure-Requests: 1
// User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/131.0.0.0 Safari/537.36 Edg/131.0.0.0
// Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.7
// Accept-Encoding: gzip, deflate
// Accept-Language: zh-CN,zh;q=0.9,en;q=0.8,en-GB;q=0.7,en-US;q=0.6

//处理http请求

//处理一行数据 
int readline(char* allbuf, int idx, char* linebuf){

    int len = strlen(allbuf);

    for(; idx < len; idx++){
        if(allbuf[idx] == '\r' && allbuf[idx + 1] == '\n'){
            return idx + 2;
        }else{
            *(linebuf++) = allbuf[idx];
        }
    }

    return -1;
}

int zv_http_response(zv_connect_t* conn){
#if 0
    int len = sprintf(conn->wbuffer, 
    "HTTP/1.1 200 OK\r\n"
    "Accept-Ranges: bytes\r\n"
    "Content-Length: 78\r\n"
    "Content-Type: text/html\r\n"
    "Date: 2024年 11月 25日 星期一 16:10:29 CST\r\n\r\n"
    "<html><head><title>moyolove.com</title></head><body><h1>Moyo<h1></body></html>"
    );

    conn->wc = len;
#elif 0
    //带html文件的send
    int filefd = open(conn->resource, O_RDONLY);
    if(filefd == -1) {
        std::cout << "open failed" << std::endl;
        return -1;
    }
    struct stat stat_buf;
    fstat(filefd, &stat_buf);
    int len = sprintf(conn->wbuffer, 
        "HTTP/1.1 200 OK\r\n"
        "Accept-Ranges: bytes\r\n"
        "Content-Length: %ld\r\n"
        "Content-Type: text/html\r\n"
        "Date: 2024年 11月 25日 星期一 16:10:29 CST\r\n\r\n", stat_buf.st_size
    );

    len += read(filefd, conn->wbuffer + len, BUFFER_SIZE - len);
    conn->wc = len;

    close(filefd);

#else
    //send_file
    int filefd = open(conn->resource, O_RDONLY);
    if(filefd == -1) {
        std::cout << "open failed" << std::endl;
        return -1;
    }
    struct stat stat_buf;
    fstat(filefd, &stat_buf);
    close(filefd);

    int len = sprintf(conn->wbuffer, 
        "HTTP/1.1 200 OK\r\n"
        "Accept-Ranges: bytes\r\n"
        "Content-Length: %ld\r\n"
        "Content-Type: text/html\r\n"
        "Date: 2024年 11月 25日 星期一 16:10:29 CST\r\n\r\n", stat_buf.st_size
    );
    conn->enable_sendfile = 1;

    conn->wc = len;
#endif
}

int zv_http_request(zv_connect_t* conn){

    std::cout << "http request" << std::endl;
    std::cout << conn->rbuffer << std::endl;

    char* linebuf = new char[BUFFER_SIZE];
    int idx = readline(conn->rbuffer, 0, linebuf);
    std::cout << "line: " << linebuf << std::endl;

    if(strstr(linebuf, "GET")){
        int i = 5;
        while(*(linebuf + i) != ' ') i++;
        *(linebuf + i) = '\0';
        std::cout << "resource: " << linebuf << std::endl;
        memcpy(conn->resource, linebuf + 4, strlen(linebuf) - 4);
        std::cout <<  conn->resource << std::endl;
    }
    while(idx != -1){
        idx = readline(conn->rbuffer, idx, linebuf);
        char* key = linebuf;
        int i = 0;
        while(key[i] != ':') i++;
        key[i] = '\0';
        char *value = linebuf + i + 1;
        put_kvpair(conn->header, key, value);

    }
    return 0;
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
    
    zv_http_response(conn);

    send(fd, conn->wbuffer, conn->wc, 0);

#if 1
    if(conn->enable_sendfile){

        int filefd = open(conn->resource, O_RDONLY);
        if(filefd == -1) {
            std::cout << "open failed" << std::endl;
            return -1;
        }
        struct stat stat_buf;
        fstat(filefd, &stat_buf);
        sendfile(fd, filefd, NULL, stat_buf.st_size);
        close(filefd);
    }
#endif

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
    
    conn->rc += ret;
    zv_http_request(conn);

    conn->cb = send_cb;

    epoll_event ev;
    ev.data.fd = fd;
    ev.events = EPOLLOUT;
    epoll_ctl(reactor->epfd, EPOLL_CTL_MOD, fd, &ev);

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
    conn->count = 1024;
    conn->header = new zv_kvstore_t();
    init_kvstore(conn->header);
    
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

    zv_reactor_t reactor;

    zv_init_reactor(&reactor);

    short port = atoi(argv[1]);
    //监听100个端口
    int i = 0;
    for(i = 0; i < 100; i++){
        int socketfd = init_server(port + i);
        set_listener(socketfd, &reactor, accept_cb);//当listened fd有事件时，触发回调accept_cb
    }


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