#pragma once
#ifndef UDPSERVER_H_
#define UDPSERVER_H_

#include <SFML/Network.hpp>
#include "driverlog.h"

// Nlohmann's JSON library
#include "nlohmann/json.hpp"

// Devices positions and orientations

struct Vector3
{
    Vector3() : x(0), y(0), z(0) {}

    Vector3(float x, float y, float z) : x(x), y(y), z(z) {}

    float x;
    float y;
    float z;
};


struct deviceData
{

    deviceData() {
        deviceName = "null";
        postion = Vector3();
        rotation = Vector3();
    }

    std::string deviceName;
    Vector3 postion;
    Vector3 rotation; // pitch; yaw; roll;
    //int buttons;
};

// json namespace
using json = nlohmann::json;

// Server class

//Data
inline deviceData HMD;
inline deviceData RightController;
inline deviceData LeftController;

inline deviceData GetDeviceData(int x) {
    switch (x)
    {
    case 0:
        return HMD;
        break;
        
    case 1:
        return RightController;
        break;

    case 2:
        return LeftController;
        break;
    }
}

class UpdServer
{
public:
    // Creating a listener for the server
    UpdServer()
    {
        socket = new sf::UdpSocket();
        if(socket->bind(50150) != sf::Socket::Done) {
            DriverLog("Error binding socket");
            // printf("Error binding socket");
        } else {
            DriverLog("Socket binded");
            // printf("Socket binded");
        }
    }
    
    // Clearing the memory after the server
    ~UpdServer()
    {
        socket->unbind();
        delete socket;
        socket = NULL;
    }

    // Main loop of the server
    void run()
    {
        while(true) {
            if(socket->receive(buffer, sizeof(buffer), received, sender, port) != sf::Socket::Done) {
                // printf("Error receiving data");
                DriverLog("Error receiving data");
            } else {
                // printf("Received data: %s\n", buffer);
                DriverLog("Received data: %s\n", buffer);

                // Parsing the data
                // { "HeadX": 0, "HeadY": 0, "HeadZ": 0, "RightX": 0, "RightY": 0, "RightZ": 0, "LeftX": 0, "LeftY": 0, "LeftZ": 0 }
                auto j3 = json::parse(buffer);

                std::fill_n(buffer, 1024, 0); // Clearing the buffer

                HMD.postion = Vector3(j3["HeadX"].get<float>(), j3["HeadY"].get<float>(), j3["HeadZ"].get<float>());
                RightController.postion = Vector3(j3["RightX"].get<float>(), j3["RightY"].get<float>(), j3["RightZ"].get<float>());
                LeftController.postion = Vector3(j3["LeftX"].get<float>(), j3["LeftY"].get<float>(), j3["LeftZ"].get<float>());

            }
        }
    }

private:
    sf::UdpSocket* socket;
    unsigned short port;

    // Sender
    sf::IpAddress sender;

    // Content
    char buffer[1024];
    std::size_t received = 0;

};

#endif // UDPSERVER_H_