#include "udp_client.h"
#define BUFFER_SIZE 2048

using namespace std;
namespace elma {
    Udp_Client::Udp_Client(){}
    void Udp_Client::sendMessage(int port, string message, function<void(string)> handler) {
        int socketFileDescriptor;
        struct sockaddr_in serverAddress;
        if ((socketFileDescriptor = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
            throw(Exception(("cannot create socket")));
        }
        //fill in the server's address and data
        memset((char *)&serverAddress, 0, sizeof(serverAddress));
        serverAddress.sin_family = AF_INET;
        serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
        serverAddress.sin_port = htons(port);
        // Convert IPv4 and IPv6 addresses from text to binary form 
        if(inet_pton(AF_INET, "127.0.0.1", &serverAddress.sin_addr)<=0) { 
            throw(Exception(("Invalid address/ Address not supported")));
        } 
        if ( sendto(socketFileDescriptor, message.c_str(), message.size(), 
            0, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
                throw(Exception(("sendto failed")));
        }
        unsigned char buffer[BUFFER_SIZE];   /* receive buffer */
        socklen_t addrlen = sizeof(serverAddress); /* length of addresses */
        int recvlen = recvfrom(socketFileDescriptor, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&serverAddress, &addrlen);
        if (recvlen > 0) {
            buffer[recvlen] = 0;
            string messageString(buffer, buffer + recvlen);
            handler(messageString);
        } else if (recvlen < 0) {
            throw "recvfrom failed";
        }
    }
}