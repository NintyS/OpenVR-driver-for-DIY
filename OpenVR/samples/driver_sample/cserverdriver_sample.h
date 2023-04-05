#ifndef CSERVERDRIVER_SAMPLE_H
#define CSERVERDRIVER_SAMPLE_H

#include <thread>
#include "driverlog.h"

#include <atomic>
#include "struct.h"
#include <iostream>

#include <openvr_driver.h>
#include <nlohmann/json.hpp>
#include "csampledevicedriver.h"
#include "csamplecontrollerdriver.h"

#include <SFML/Window/Keyboard.hpp>
#include <SFML/Network.hpp>

using json = nlohmann::json;

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
class CServerDriver_Sample : public vr::IServerTrackedDeviceProvider
{
public:
    virtual vr::EVRInitError Init(vr::IVRDriverContext *pDriverContext);
    virtual void Cleanup();
    virtual const char *const *GetInterfaceVersions() { return vr::k_InterfaceVersions; }
    virtual void RunFrame();
    virtual bool ShouldBlockStandbyMode()  { return false; }
    virtual void EnterStandby()  {}
    virtual void LeaveStandby()  {}

    void receive();

    // Sending events to the client
    void sendEvents();

    // Sending the image to the client
    void sendImage();

    // Main loop of the server
    void setUp();

    Point3D &getHMDPosition(); // Get the position of the HMD

    Point3D &getControllerPosition(int controller); // Get the position of the controller

    Buttons &getButtons(int controller); // Get the buttons of the controller

    void unJSON(std::string &jsonStr);

    std::string createJSON(std::map<std::string, std::string> &values);

    void GetImage();

private:
    CSampleDeviceDriver *m_pNullHmdLatest = nullptr;
    CSampleControllerDriver *m_pController = nullptr;
    CSampleControllerDriver *m_pController2 = nullptr;

    //Network
    sf::UdpSocket* receiveSocket;
    sf::UdpSocket* sendEventsSocket;
    sf::UdpSocket* sendImageSocket;
    unsigned short port = 50153;

    // Sender
    sf::IpAddress sender; // Ip of the sender to send the data

    // Threads
    std::thread *MainThread; // Main thread of the server
    std::thread *ReceiveThread; // Receive a HMD and controllers position.
    std::thread *SendEventsThread; // Send events to the client. (Haptic events, etc.)
    std::thread *SendImageThread; // Send the image to the client.

    // Controllers of thread
//        bool MainThreadRunning = true;
        std::atomic<bool> ReceiveThreadRunning = true;
        std::atomic<bool> SendEventsThreadRunning = true;
        std::atomic<bool> SendImageThreadRunning = true;

    // Content
    char positionBuffer[2048];
    std::size_t p_received = 0;

    char eventsBuffer[1024];
//    std::size_t e_received = 0;

    char imageBuffer[1024];
//    std::size_t i_received = 0;

    // Devices
    Point3D HMDPosition;
    Point3D ControllerPosition[2];
    Buttons ControllerButtons[2];

};

#endif // CSERVERDRIVER_SAMPLE_H
