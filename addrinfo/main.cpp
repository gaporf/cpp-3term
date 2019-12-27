#include "server.h"
#include "epoll_raii.h"

int main() {
    epoll_raii er;
    server f(3238, er);
    server g(4000, er);
    while (true) {
        er.execute();
    }
    return 0;
}
