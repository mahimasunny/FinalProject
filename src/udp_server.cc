#include "elma.h"
#include "json/json.h"
#include "udp_server.h"
#include <stdexcept>
#define BUFFER_SIZE 2048

using namespace elma;
using namespace std;

Udp_Server::Udp_Server(int port) {
    /* create a UDP socket */
    struct sockaddr_in _addr; /* our address */
    if ((_server_file_descriptor = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        throw "cannot create socket";
    }
    _port = port;
    /* bind the socket to any valid IP address and a specific port */
    memset((char *)&_addr, 0, sizeof(_addr));
    _addr.sin_family = AF_INET;
    _addr.sin_addr.s_addr = htonl(INADDR_ANY);
    _addr.sin_port = htons(_port);
    try {
        bind(_server_file_descriptor, (struct sockaddr *)&_addr, sizeof(_addr));
    }
    catch (const std::exception &e) {
        std::cout<<"bind failed"<<std::endl;
    }
}

void Udp_Server::start(function<string(string)> handler) {
    struct sockaddr_in remaddr;          /* remote address */
    socklen_t addrlen = sizeof(remaddr); /* length of addresses */
    int recvlen;                         /* # bytes received */
    unsigned char buffer[BUFFER_SIZE];   /* receive buffer */
    /* now loop, receiving data and printing what we received */
    for (;;) {
        cout << "waiting on port " << _port << endl;
        recvlen = recvfrom(_server_file_descriptor, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&remaddr, &addrlen);
        if (recvlen > 0) {
            buffer[recvlen] = 0;
            string messageString(buffer, buffer + recvlen);
            string response = handler(messageString);
            if ( sendto(_server_file_descriptor, response.c_str(), response.size(), 
                0, (struct sockaddr *)&remaddr, sizeof(remaddr)) < 0) {
                    throw(Exception(("sendto failed")));
            }
        } else if (recvlen < 0) {
            throw "recvfrom failed";
        }
    }
}