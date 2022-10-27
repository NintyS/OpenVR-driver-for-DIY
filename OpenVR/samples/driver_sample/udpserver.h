#pragma once
#ifndef UDPSERVER_H
#define UDPSERVER_H

#include <SFML/Network.hpp>
#include "driverlog.h"

// Devices positions and orientations

struct Vector3
{
    float x;
    float y;
    float z;
};


struct deviceData
{
    std::string deviceName;
    Vector3 postion;
    Vector3 rotation; // pitch; yaw; roll;
    //int buttons;
};

// Server class

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
                parseJson(buffer);

                std::fill_n(buffer, 1024, 0); // Clearing the buffer
            }
        }
    }

    // Function that serialize JSON to struct
    // { "HeadX": 0, "HeadY": 0, "HeadZ": 0, "RightX": 0, "RightY": 0, "RightZ": 0, "LeftX": 0, "LeftY": 0, "LeftZ": 0 }
    std::vector<Vector3> parseJson(std::string json) 
    {
        json = json.substr(1, json.length() - 2); // Removing brackets

        for (int i = 0; i < 3; i++)
        {
            Vector3 v;
            for (int i = 0; i < 3; i++)
            {
                if(json.find(",") == std::string::npos) {
                    std::string text = json.substr(0, json.length() - 1);
                }
                std::string text = json.substr(0, json.find(","));
                std::string var = text.substr(text.find(":") + 1, json.find(","));
                DriverLog("Varable: ", var);

            }
        }

        return std::vector<Vector3>();
        
    }

private:
    sf::UdpSocket* socket;
    unsigned short port;

    // Sender
    sf::IpAddress sender;

    // Content
    char buffer[1024];
    std::size_t received = 0;

    //Data
    deviceData HMD;
    deviceData RightController;
    deviceData LeftController;
};

#endif // UDPSERVER_H
