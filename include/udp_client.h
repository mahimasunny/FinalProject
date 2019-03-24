#ifndef _UDP_CLIENT_H
#define _UDP_CLIENT_H

#include <sys/types.h>
#include <sys/socket.h>
#include <string>
#include <netinet/in.h>
// #include <arpa/inet.h>
// #include <netdb.h>
#include <iostream>
#include <cstring>
#include <stdexcept>
#include "elma.h"
using namespace std;

//! UDP Client class
namespace elma {
    //! It sends messages to the UDP Server.
    class Udp_Client {
    public:
        //! Construct a UDP Client
        Udp_Client();   
        
        //! Send message to thespecified ports 
        void sendMessage(int port_send, string message, function<void(string)> handler);
    };
}
#endif