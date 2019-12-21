#include "server.h"
#include "epoll_raii.h"

int main() {
    epoll_raii er;
    server f(3234);
    er.add_server(f);
    er.execute();
    return 0;
}
