
# Heater Implementation with a Remote Thermostat Using UDP
===


Introduction
===
User Datagram Protocol(UDP) is used in internet applications to implement low-latency and loss-tolerating commenction. UDP enalbles a process to process communication. This project aims to implement a bidirectional UDP message transferring mechanism between a UDP client and a UDP Server.


Project Details
===
This project is a UDP based iOT application that comprises of a Thermostat(UDP Server) and a Room Heater (UDP client). The thermostat and the heater communicate to each other via UDP.  There is also an internal manager in heater side that controls the heater, that is turn the heater on or off. 
 The Thermostat can send the current temperature value to the heater in JSON format. The Thermostat listens to a predefined port for the messages from the Heater.  
 
 Heater is a finite state machine that has an on state and an off state. It is initially set to an off state. 
The internal Manager sends a message to turn on the heater. If the heater is on, it requests for a temperature value from the thermostat. Thermostat then sends back the requested temperature value.


Installation
===

The source code for Elma [is on Github](https://github.com/mahimasunny/FinalProject).

From the Dockerfile
---

You can build the docker environment, described in env/Dockerfile, yourself, with the following commands:

```
    git clone https://github.com/mahimasunny/FinalProject.git
    cd FinalProject
    docker build -t myelma .
    docker run -v $PWD:/source -it myelma bash
    make
    ./examples/bin/heater
    ./examples/bin/thermostat
```

Implementation
===

*Steps in implementing UDP Server and Client with sockets*
---
The following are the steps I followed for the completion of this project.
1.  Socket creation    
	```
	#include <sys/socket.h>

	...

	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("cannot create socket");
		return 0;
	}

	```
	where, sockfd is the socket descripton, domain refers to communication domain for example AF_INET (IPv4 protocol) , AF_INET6 (IPv6 protocol), type is SOCK_DGRAM for UDP and protocol value is the value for Internet Protocol which is 0.
	Here all the input parameters are integers. socket mehod is defined in sys/socket.h.

2.  Identify a socket
	Sockets are assigned port numbers or transport addresses.
	
	In server side
	```
	struct sockaddr_in myaddr;

	/* bind to an arbitrary return address */
	/* because this is the client side, we don't care about the address */
	/* since no application will initiate communication here - it will */
	/* just send responses */
	/* INADDR_ANY is the IP address and 0 is the socket */
	/* htonl converts a long integer (e.g. address) to a network representation */
	/* htons converts a short integer (e.g. port) to a network representation */

	memset((char *)&myaddr, 0, sizeof(myaddr));
	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	myaddr.sin_port = htons(PORT);

	if (bind(fd, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
		perror("bind failed");
		return 0;
	}
	```
	In the server side, Bind method forcefully binds the PORT address.
	
	
	Client side: send message to the PORT to which binding was done at the server side.
	```
	memset((char *)&myaddr, 0, sizeof(myaddr));
	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	myaddr.sin_port = htons(0);
	```

3. 	Send message to a server from a client   
	a. Client side   
	 sendto is used to send a message from client to server

	```
	sendto(fd, buf, strlen(buf), 0, (struct sockaddr *)&remaddr, slen);
	```
	fd refers to the name of the socket. 
	my_message, provides the starting address of the message we want to send.
	The flags parameter is 0 for UDP sockets. 
	The remaddr defines the destination address and port number for the message. socklen_t is the length of the address structure: sizeof(struct sockaddr_in).

	b. Receive message at server     
	```
	/* now loop, receiving data and printing what we received */
        for (;;) {
                printf("waiting on port %d\n", PORT);
                recvlen = recvfrom(fd, buf, BUFSIZE, 0, (struct sockaddr *)&remaddr, &addrlen);
                printf("received %d bytes\n", recvlen);
                if (recvlen > 0) {
                        buf[recvlen] = 0;
                        printf("received message: \"%s\"\n", buf);
                }
        }
        /* never exits */

	```
	fd refers to the name of the socket.
	buf is the input data and recvlen is the length of that data
	The flags parameter is 0 for UDP sockets. 
	remaddr is used to recognize the sender.


4.  On the client, send a message    

	Message from client to server    
	```
	recvlen = recvfrom(s, buf, BUFSIZE, 0, (struct sockaddr *)&remaddr, &addrlen);
	```

	Reply from server to client send to remaddr 
	```
	sendto(s, buf, strlen(buf), 0, (struct sockaddr *)&remaddr, addrlen)

	```

	At this stage the client can also send back a message to the server


5.  Close the socket  
	Eventhough there is no communication to be terminated in UDP, the system needs to be closed to free up memory.
	```
	close(fd);

	```
	fd refers to the file descriptor 
	
	
*Steps in implementing Thermostat(UDP Server) and Heater(UDP Client)*
---
1. Thermostat listens to a predefined port for the messages from the Heater.  
2. The internal Manager of the heater sends a message to turn on the heater. 
3.  If the heater is on, it requests for a temperature value from the thermostat.  
4. Thermostat sends back the requested temperature value  in JSON format. 

**Heater class**
```
class Heater : public Process {

        public:

        //! Create a new Heater component
        Heater(int thermostatPort) : Process("roomHeater") {
            port = thermostatPort;
            temperature = -1;
            running = true;
        }

        //! Set up watches for three events: "on", "off", and "set temperature".
        //! The "set temperature" watcher expects events whose values are numerical.
        void init() {
            watch("on", [this](Event& e) {
                start();  
            });
            watch("off", [this](Event &e) {
                stop();
            });
            watch("set temperature", [this](Event& e) {
                temperature = e.value();
                log("Set heater temperature to " + to_string(temperature));
            });
        }

        void log(string logText) {
            cout<<"logging : "<< logText<<endl;
        }

        //! Set running to true
        void start() {
            // running = true;
            update();
            log("Heater started");
        }

        void stop() {
            running = false;
            log("Heater stopped");
        }

        void update() {
            if(running) {
                Udp_Client sender;
                sender.sendMessage(port, "{\"operation\" : \"temperature\"}", [this](string message){
                    json mesageJson = json::parse(message);
                    cout<<mesageJson.dump()<<endl;
                    if (mesageJson["value"].is_number()) {
                        temperature = mesageJson["value"];
                    }
                    cout<<temperature<<endl;
                });
            }
        }
        bool running;
        int temperature;
        private:
        int port;

    };
```

**Thermostat class**
```
class Thermostat {

        public:
        Thermostat() {
            temperature = 25;
        }

        int getTemperature() {
            return temperature++;
        }
        private:
        int temperature;
    };
int main(int argc, char const *argv[]) {
    //! Start the server and listen for temperature reading 
    int port = 8000; //Default port
    if(argv[1]) {
        port = atoi(argv[1]);
    }
    thermostat_example::Thermostat roomThermostat;
    Udp_Server server(port);
    server.start([&roomThermostat](string message) {
        json mesageJson = json::parse(message);
        string temp;
        if (mesageJson["operation"] == "temperature") {
            temp = to_string(roomThermostat.getTemperature());
        }
        return "{\"value\" : " + temp + "}";
    });
}
```

Resources used
===
*Libraries*
Elma  
C++ libararies such as iostream, json/json.h, map, string 
sys/socket.h  
netdb.h 
arpa/inet.h 

*Other References*
https://www.cs.rutgers.edu/~pxk/417/notes/sockets/udp.html
https://www.geeksforgeeks.org/socket-programming-cc/
https://pubs.opengroup.org/onlinepubs/7908799/xns/syssocket.h.html








