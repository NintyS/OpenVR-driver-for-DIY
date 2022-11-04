#pragma once
#ifndef UDPSERVER_H_
#define UDPSERVER_H_

#include <SFML/Network.hpp>
#include "driverlog.h"
#include <string>

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

    deviceData(std::string deviceName, Vector3 postion, Vector3 rotation) {
        this->deviceName = deviceName;
        this->postion = postion;
        this->rotation = rotation;
    }

    std::string deviceName;
    Vector3 postion;
    Vector3 rotation; // pitch; yaw; roll;
    //int buttons;
};
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

    default:
        DriverLog("Error: GetDeviceData() - Invalid device index");
        return deviceData();
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
                // { "HeadX": 0, "HeadY": 0, "HeadZ": 0, "RightX": 0, "RightY": 0, "RightZ": 0, "LeftX": 0, "LeftY": 0, "LeftZ": 0 } // changed from json to CSV
                std::vector<float> data = divideString(buffer);

                std::fill_n(buffer, 1024, 0); // Clearing the buffer

                // // DriverLog("HeadX: %f", j3["HeadX"]);
                // // DriverLog("HeadY: %f", j3["HeadY"]);
                // // DriverLog("HeadZ: %f", j3["HeadZ"]);

                if(!data.empty()) {
                    for( auto i : data) {
                        DriverLog("Value: %f", i);
                    }

                    HMD.postion = Vector3(data[0], data[1], data[2]);
                    RightController.postion = Vector3(data[3], data[4], data[5]);
                    LeftController.postion = Vector3(data[6], data[7], data[8]);
                }
            }
        }
    }

    std::vector<float> divideString(std::string str) {

        std::vector<float> dividedString;
        std::string tempString = "";

        for (int i = 0; i < str.length(); i++) {
            if (str[i] == '|') {
                dividedString.push_back(std::stof(tempString));
                tempString = "";
            }
            else {
                tempString += str[i];
            }
        }

        return dividedString;

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