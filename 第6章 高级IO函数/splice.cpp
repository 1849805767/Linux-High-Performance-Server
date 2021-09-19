#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cassert>
#include <cstdio>
#include <unistd.h>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <thread>
using namespace std;

int main(int argc, char* argv[]) {
    if(argc <= 2) {
        printf("usage: %s ip_address port_number\n", basename(argv[0]));
        return 1;
    }
    const char* ip = argv[1];
    int port = atoi(argv[2]);

    sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &address.sin_addr);
    address.sin_port = htons(port);

    int sock = socket(PF_INET, SOCK_STREAM, 0);
    assert(sock >= 0);

    int ret = bind(sock, (sockaddr*)&address, sizeof(address));
    assert(ret != -1);

    ret = listen(sock, 5);
    assert(ret != -1);

    sockaddr_in client;
    socklen_t client_addrlength = sizeof(client);
    int connfd = accept(sock, (sockaddr*)&client, &client_addrlength);
    if(connfd < 0) {
        printf("errno is: %d\n", errno);
    }
    else {
        int pipefd[2];
        ret = pipe(pipefd); // 创建管道
        assert(ret != -1);
        while(1) {
            // 将connfd上流入的客户端数据定向到管道中
            ret = splice(connfd, nullptr, pipefd[1], nullptr, 10, SPLICE_F_MORE | SPLICE_F_MOVE);
            assert(ret != -1);
            // 将管道的输出定向到connfd客户连接文件描述符
            ret = splice(pipefd[0], nullptr, connfd, nullptr, 10, SPLICE_F_MORE | SPLICE_F_MOVE);
            assert(ret != -1);
            // printf("------\n");
        }
        close(connfd);
    }
    close(sock);

    return 0;
}