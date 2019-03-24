#ifndef _UDP_SERVER_H
#define _UDP_SERVER_H

#include <sys/types.h>
#include <sys/socket.h>
#include <string>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <iostream>
#include <cstring>
#include <stdexcept>

#include "elma.h"
using namespace std;

//! UDP Server Class
//! This UDS server listens on an assigned port and parse all the meddages as json and call the handler lambda function
namespace elma {
    class Udp_Server {
        public:
            //! Construct a new UDP Server
            Udp_Server(int port);
            //! starts the server
            void start(function<string(string)> handler);
        private:
            //! socket name
            int _server_file_descriptor;

            //! port id
            int _port;
    };
}
#endif