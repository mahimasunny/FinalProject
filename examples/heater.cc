#include <iostream>
#include <chrono>
#include "elma.h"
#include "json/json.h"
#include "gtest/gtest.h"

//! \file

using namespace std::chrono;
using namespace elma;

namespace heater_example {

    //! A class the models the Heater functionality
    //! Heater is the UDP Server in our example
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

    //! The state in which the Heater is off.
    class HeaterOff : public State {

        public:
        //! Make a new state
        HeaterOff() : State("Heater off") {}

        //! If this state is entered via a "power level set" event
        //! from the user, then emit a "set power" event (to be)
        //! consumed by the power component.
        //! \param e A numerically valued event
        void entry(const Event& e) {
            if ( e.name() == "Temperature set" ) {
                emit(Event("set temperature", e.value()));
            }
        }

        //! Nothing to do in during
        void during() {} 

        //! If this state is left via an "on button pressed" event,
        //! then emit a "on" event.
        //! \param e An event with no value
        void exit(const Event& e) {
            if ( e.name() == "on button pressed" ) {
                emit(Event("on"));
            }        
        }
    };

    //! The state in which the Heater is on.
    class HeaterOn : public State {
        public:
        //! Make a new state
        HeaterOn() : State("Heater on") {}

        //! Nothing to do upon entry        
        void entry(const Event& e) {
            if ( e.name() == "Temperature set" ) {
                emit(Event("set temperature", e.value()));
            }
        }

        //! Nothing to do in during 
        void during() {} 

        //! As this state is left, turn off the oven.
        //! \param e An event with no value        
        void exit(const Event& e) {
            if ( e.name() == "off button pressed" ) {
                emit(Event("off"));
            }
        }
    };
}

TEST(Heater, ManagerInterface) {
    int port = 8000; //Default port
    Manager m;
    heater_example::Heater roomHeater(port);
    heater_example::HeaterOff heater_off;
    heater_example::HeaterOn heater_on;

    StateMachine user;
    user
      .set_initial(heater_off)
      .add_transition("on button pressed", heater_off, heater_on)
      .add_transition("off button pressed", heater_on, heater_off)
      .add_transition("Temperature set", heater_off, heater_off)
      .add_transition("Temperature set", heater_on, heater_on);

    m.schedule(roomHeater, 10_ms)
      .schedule(user, 10_ms)
      .init();
    
    m.start();
    m.emit(Event("on button pressed"));
    ASSERT_EQ(true, roomHeater.running);
    ASSERT_NE(-1, roomHeater.temperature); 

    m.emit(Event("off button pressed"));    
    ASSERT_EQ(false, roomHeater.running); 

    m.emit(Event("Temperature set", 30));
    ASSERT_EQ(30, roomHeater.temperature); 

    m.stop();

}

TEST(UDP, Communicatiom){
    int port = 8000; //Default port
    Manager m;
    heater_example::Heater roomHeater(port);
    heater_example::HeaterOff heater_off;
    heater_example::HeaterOn heater_on;

    m.schedule(roomHeater, 500_ms)
      .init()
      .start()
      .run(2_s);
    ASSERT_NE(-1, roomHeater.temperature);
}

GTEST_API_ int main(int argc, char **argv) {


  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}