#include <iostream>
#include <string.h>
#include <SFML/Network.hpp>

#include <chrono>
#include <thread>

int main() {
    // Connect VIA UPD to the server to test position and rotation
    sf::UdpSocket socket;
    sf::IpAddress ip = "127.0.0.1";
    unsigned short port = 50150;

    // Positions
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;

    // Send a message to the server
    while (true)
    {  
        std::string message = std::to_string(x) + "|" + std::to_string(y) + "|" + std::to_string(z) + "|0|0|0|0|0|0|";
        if (socket.send(message.c_str(), message.length() + 1, ip, port) != sf::Socket::Done)
        {
            std::cout << "Error sending message" << std::endl;
        } else {
            std::cout << "Message sent" << std::endl;
        }
        x += 0.1f;
        y += 0.1f;
        z += 0.1f;
        
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

    }

}