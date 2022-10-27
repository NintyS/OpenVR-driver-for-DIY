#pragma once
#ifndef UDPSERVER_H
#define UDPSERVER_H

#include <SFML/Network.hpp>
#include "driverlog.h"

// Devices positions and orientations

struct deviceData
{
    std::string deviceName;
    int x;
    int y;
    int z;
    int pitch;
    int yaw;
    int roll;
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
        DriverLog("Server started");
        while(true) {
            if(socket->receive(buffer, sizeof(buffer), received, sender, port) != sf::Socket::Done) {
                // printf("Error receiving data");
                DriverLog("Error receiving data");
            } else {
                // printf("Received data: %s\n", buffer);
                DriverLog("Received data: %s\n", buffer);
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

    //Data
    deviceData HMD;
    deviceData RightController;
    deviceData LeftController;
};

#endif // UDPSERVER_H
