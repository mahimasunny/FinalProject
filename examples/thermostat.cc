#include <iostream>
#include "elma.h"
#include "json/json.h"

//! \file

using namespace std::chrono;
using namespace elma;

namespace thermostat_example {

    //! Thermostat implementation
    //! Thermostat is the UDP Server in our example
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
}

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