#include "cserverdriver_sample.h"

using namespace vr;

EVRInitError CServerDriver_Sample::Init(vr::IVRDriverContext *pDriverContext) {
    VR_INIT_SERVER_DRIVER_CONTEXT(pDriverContext);
    InitDriverLog(vr::VRDriverLog());

    m_pNullHmdLatest = new CSampleDeviceDriver();
    vr::VRServerDriverHost()->TrackedDeviceAdded(m_pNullHmdLatest->GetSerialNumber().c_str(),
                                                 vr::TrackedDeviceClass_HMD, m_pNullHmdLatest);

    m_pController = new CSampleControllerDriver();
    m_pController->SetControllerIndex(1);
    vr::VRServerDriverHost()->TrackedDeviceAdded(m_pController->GetSerialNumber().c_str(),
                                                 vr::TrackedDeviceClass_Controller, m_pController);

    m_pController2 = new CSampleControllerDriver();
    m_pController2->SetControllerIndex(2);
    vr::VRServerDriverHost()->TrackedDeviceAdded(m_pController2->GetSerialNumber().c_str(),
                                                 vr::TrackedDeviceClass_Controller, m_pController2);

//    m_pNullDisplayLatest = new CSampleDisplayDevice();
//    vr::VRServerDriverHost()->TrackedDeviceAdded(std::string("VirtualDisplay").c_str(),
//                                                 vr::TrackedDeviceClass_DisplayRedirect, m_pNullDisplayLatest);

    std::cout << "\033[31m" << "ServerDriver_Sample::Init" << "\033[0m" << std::endl;

    try {
        receiveSocket = new sf::UdpSocket();
        if (!receiveSocket->bind(50153)) {
            std::cout << "Error binding socket - receiveSocket" << std::endl;
        } else {
            std::cout << "Socket binded - receiveSocket" << std::endl;
        }

        sendEventsSocket = new sf::UdpSocket();
//        if (!sendEventsSocket->bind(50154)) {
//            std::cout << "Error binding socket - sendEventsSocket" << std::endl;
//        } else {
//            std::cout << "Socket binded - sendEventsSocket" << std::endl;
//        }
//
        sendImageSocket = new sf::UdpSocket();
//        if (!sendImageSocket->bind(50155)) {
//            std::cout << "Error binding socket - sendImageSocket" << std::endl;
//        } else {
//            std::cout << "Socket binded - sendImageSocket" << std::endl;
//        }

    } catch (std::exception &e) {
        std::cout << "Error creating sockets: " << e.what() << std::endl;
    }

    try {
        ReceiveThread = new std::thread(&CServerDriver_Sample::receive, this);
//        SendEventsThread = new std::thread(&CServerDriver_Sample::sendEvents, this);
        SendImageThread = new std::thread(&CServerDriver_Sample::sendImage, this);

        ReceiveThread->detach();
//        SendEventsThread->detach();
        SendImageThread->detach();

    } catch (std::exception &e) {
        std::cout << "Error creating threads: " << e.what() << std::endl;
    }

    std::cout << "\033[31m End of Init\033[0m" << std::endl;

    return VRInitError_None;
}

void CServerDriver_Sample::Cleanup()
{
    delete m_pNullHmdLatest;
    m_pNullHmdLatest = NULL;
    delete m_pController;
    m_pController = NULL;
    delete m_pController2;
    m_pController2 = NULL;

    std::cout << "Start od deleting the server" << std::endl;

    // Stopping the threads
    if(ReceiveThreadRunning.exchange(false)) {
        std::cout << "ReceiveThreadRunning = false" << std::endl;
        ReceiveThread->join();
    }

    if(SendEventsThreadRunning.exchange(false)) {
        std::cout << "SendEventsThreadRunning = false" << std::endl;
        SendEventsThread->join();
    }

    if(SendImageThreadRunning.exchange(false)) {
        std::cout << "SendImageThreadRunning = false" << std::endl;
        SendImageThread->join();
    }

    // Closing the socket
    receiveSocket->unbind();
    sendEventsSocket->unbind();
    sendImageSocket->unbind();

    // Deleting the sockets
    delete receiveSocket;
    receiveSocket = NULL;
    delete sendEventsSocket;
    sendEventsSocket = NULL;
    delete sendImageSocket;
    sendImageSocket = NULL;

    // Deleting the threads
    delete ReceiveThread;
    ReceiveThread = NULL;
    delete SendEventsThread;
    SendEventsThread = NULL;
    delete SendImageThread;
    SendImageThread = NULL;
    delete MainThread;
    MainThread = NULL;
    std::cout << "End of deleting the server" << std::endl;

}

void CServerDriver_Sample::RunFrame()
{
//    std::cout << "\033[31m" << "ServerDriver_Sample::RunFrame" << "\033[0m" << std::endl;

    try {
        if (m_pNullHmdLatest) {
            m_pNullHmdLatest->SetPosition(getHMDPosition());
            m_pNullHmdLatest->RunFrame();
        }

        if (m_pController) {
            m_pController->SetPositions(getControllerPosition(0), getHMDPosition());
            m_pController->SetButtons(getButtons(0));
            m_pController->RunFrame();
        }

        if (m_pController2) {
            m_pController2->SetPositions(getControllerPosition(1), getHMDPosition());
            m_pController2->SetButtons(getButtons(1));
            m_pController2->RunFrame();
        }
    } catch (std::exception &e) {
        std::cout << "\033[31m Error in RunFrame: " << e.what() << "\033[0m" << std::endl;
    }

#if 0
    vr::VREvent_t vrEvent;
    while ( vr::VRServerDriverHost()->PollNextEvent( &vrEvent, sizeof( vrEvent ) ) )
    {
        if ( m_pController )
        {
            m_pController->ProcessEvent(vrEvent);
        }
        if ( m_pController2)
        {
            m_pController2->ProcessEvent(vrEvent);
        }
    }
#endif
}

void CServerDriver_Sample::receive() {
    try {
        while (ReceiveThreadRunning) {
            if (receiveSocket->receive(positionBuffer, sizeof(positionBuffer), p_received, sender, portOne) !=
                sf::Socket::Done) {
                std::cout << "Error receiving data" << std::endl;
            } else {
                std::cout << "Sender - receive: " << sender << std::endl;
                if(sender.toString() != "0.0.0.0") {
                    receiver = sender;
                    std::cout << " >>> Receiver: SAVED" << std::endl;
                } else {
                    std::cout << " >>> Receiver: NOT-SAVED" << std::endl;
                }

                std::cout << "Received data, on port: " << portOne << std::endl;
                std::cout << "Received data: " << positionBuffer << std::endl;

                std::string str = "";

                for (auto &s: positionBuffer) {
                    str += s;
                }

                unJSON(str);

                //clear positionBuffer
                for (auto &s: positionBuffer) {
                    s = 0;
                }
                std::cout << "HMD: X: " << getHMDPosition().x << " Y: " << getHMDPosition().y << " Z: "
                          << getHMDPosition().z << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
            }
        }
    } catch (std::exception &e) {
        std::cout << "Error in receive: " << e.what() << std::endl;
    }
}

//void CServerDriver_Sample::sendEvents() {
//    try {
//        while (SendEventsThreadRunning) {
//            if (sendEventsSocket->send(eventsBuffer, sizeof(eventsBuffer), sender, portTwo) != sf::Socket::Done) {
//                std::cout << "Error sending data" << std::endl;
//            } else {
//                std::cout << "Sent data" << std::endl;
//            }
//        }
//    } catch (std::exception &e) {
//        std::cout << "Error in sendEvents: " << e.what() << std::endl;
//    }
//}

void CServerDriver_Sample::sendImage() {
    try {

        sf::IpAddress sendback;

        while (sendback.toString() == "0.0.0.0") {
            if (sendback.toString() != "0.0.0.0") {
                std::cout << "Fetched" << std::endl;
                break;

            }

            if (sendback.toString() == "0.0.0.0") {
                std::cout << "Not fetched" << std::endl;
                std::cout << "Receiver: " << receiver << std::endl;
                sendback = receiver;
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
                continue;
            }
        }

        while (SendImageThreadRunning) {

            auto data = m_pNullHmdLatest->GetEyeView();
            if (data.empty()) {
                std::cout << "Data is null" << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                continue;
            }

            if(data.data == image.data) {
                continue;
            }

            std::vector<uchar> imageBuf;

            cv::imencode(".jpg", data, imageBuf);

            std::cout << imageBuf.size() << std::endl;

            const int buffer_size = 5120;  // Size of each buffer in bytes
            char buffer[buffer_size];
            int buffer_idx = 0;  // Current position in buffer

            buffer[0] = '<';
            buffer[1] = 'i';
            buffer[2] = 'm';
            buffer[3] = 'g';
            buffer[4] = '>';
            buffer_idx = 5;

            int timeOfSleep = 1;

            int packets_sended = 0;

            for (size_t i = 0; i < imageBuf.size(); ++i) {
                // Copy data to buffer
                buffer[buffer_idx++] = imageBuf[i];

                if (buffer_idx == buffer_size) {
                    // Buffer is full, send it over network
                    if (sendImageSocket->send(buffer, 5120, receiver, 50155) != sf::Socket::Done) {
                        std::cout << "Error sending packet" << std::endl;
                    } else {

                        std::memset(buffer, 0, buffer_size);
                        // Reset buffer index
                        buffer_idx = 0;

                        packets_sended++;
                        std::cout << "Packets sended: " << packets_sended << std::endl;
                        std::this_thread::sleep_for(std::chrono::milliseconds(timeOfSleep));
                    }
                }
            }

            // Send the remaining data in the buffer (if any)
            if (buffer_idx > 0) {
                if (sendImageSocket->send(buffer, buffer_idx, receiver, 50155) != sf::Socket::Done) {
                    std::cout << "Error sending packet" << std::endl;
                } else {
                    packets_sended++;
                    std::cout << "Packets sended: " << packets_sended << std::endl;
                    std::this_thread::sleep_for(std::chrono::milliseconds(timeOfSleep));
                }
                std::memset(buffer, 0, buffer_size);
                buffer_idx = 0;
            }

            buffer[0] = '<';
            buffer[1] = '/';
            buffer[2] = 'i';
            buffer[3] = 'm';
            buffer[4] = 'g';
            buffer[5] = '>';
            if (sendImageSocket->send(buffer, 5120, receiver, 50155) != sf::Socket::Done) {
                std::cout << "Error sending packet" << std::endl;
            } else {
                packets_sended++;
                std::cout << "Packets sended: " << packets_sended << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(timeOfSleep));
            }
            std::memset(buffer, 0, buffer_size);

            data = image;
            data.release();

//            data = nullptr;
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
    } catch (std::exception &e) {
        std::cout << "Error in sendImage: " << e.what() << std::endl;
    }
}

Point3D &CServerDriver_Sample::getHMDPosition() {
    return HMDPosition;
}

Point3D &CServerDriver_Sample::getControllerPosition(int controller) {
    return ControllerPosition[controller];
}

Buttons &CServerDriver_Sample::getButtons(int controller) {
    return ControllerButtons[controller];
}

void CServerDriver_Sample::unJSON(std::string &jsonStr) {
    // UnJSON
    try {
//        std::cout << " \033[31m UnmarshalJSON: " << str << " \033[0m" << std::endl;
        std::string str2 = jsonStr.substr(0, jsonStr.find("} } }") + 5);

        for (int i = 0; i < str2.length(); i++) {
            if (str2[i] == ',') {
                // Add a new line after each comma
                str2.insert(i + 1, "\n");
            }
        }

        std::cout << " \033[31m Cuted string: " << str2 << " \033[0m" << std::endl;

        auto j3 = json::parse(str2);

//        std::cout << " \033[31m JSON: " << j3 << " \033[0m" << std::endl;

        std::cout << " \033[31m First part \033[0m" << std::endl;
        std::cout << " \033[31m Head: " << j3["HMD"]["position"]["x"] << j3["HMD"]["position"]["y"] << j3["HMD"]["position"]["z"] << " \033[0m" << std::endl;
        std::cout << " \033[31m Head: " << j3["HMD"]["rotation"]["x"] << j3["HMD"]["rotation"]["y"] << j3["HMD"]["rotation"]["z"] << j3["HMD"]["rotation"]["w"] << " \033[0m" << std::endl;
        std::cout << std::endl;
        std::cout << " \033[31m Left: " << j3["LeftController"]["position"]["x"] << j3["LeftController"]["position"]["y"] << j3["LeftController"]["position"]["z"] << " \033[0m" << std::endl;
        std::cout << " \033[31m Left: " << j3["LeftController"]["rotation"]["x"] << j3["LeftController"]["rotation"]["y"] << j3["LeftController"]["rotation"]["z"] << j3["LeftController"]["rotation"]["w"] << " \033[0m" << std::endl;
        std::cout << std::endl;
        std::cout << " \033[31m Right: " << j3["RightController"]["position"]["x"] << j3["RightController"]["position"]["y"] << j3["RightController"]["position"]["z"] << " \033[0m" << std::endl;
        std::cout << " \033[31m Right: " << j3["RightController"]["rotation"]["x"] << j3["RightController"]["rotation"]["y"] << j3["RightController"]["rotation"]["z"] << j3["RightController"]["rotation"]["w"] << " \033[0m" << std::endl;

        HMDPosition = Point3D(j3["HMD"]["position"]["x"], j3["HMD"]["position"]["y"], j3["HMD"]["position"]["z"],
                              j3["HMD"]["rotation"]["x"], j3["HMD"]["rotation"]["y"], j3["HMD"]["rotation"]["z"],
                              j3["HMD"]["rotation"]["w"]);

        ControllerPosition[0] = Point3D(j3["LeftController"]["position"]["x"], j3["LeftController"]["position"]["y"],
                                        j3["LeftController"]["position"]["z"], j3["LeftController"]["rotation"]["x"],
                                        j3["LeftController"]["rotation"]["y"], j3["LeftController"]["rotation"]["z"],
                                        j3["LeftController"]["rotation"]["w"]);

        ControllerPosition[1] = Point3D(j3["RightController"]["position"]["x"], j3["RightController"]["position"]["y"],
                                        j3["RightController"]["position"]["z"], j3["RightController"]["rotation"]["x"],
                                        j3["RightController"]["rotation"]["y"], j3["RightController"]["rotation"]["z"],
                                        j3["RightController"]["rotation"]["w"]);

//        std::cout << " \033[31m Second part \033[0m" << std::endl;
//        std::cout << " \033[31m Head: " << j3["HMD"]["position"]["x"] << j3["HMD"]["position"]["y"] << j3["HMD"]["position"]["z"] << " \033[0m" << std::endl;
//        std::cout << " \033[31m Head: " << j3["HMD"]["rotation"]["x"] << j3["HMD"]["rotation"]["y"] << j3["HMD"]["rotation"]["z"] << j3["HMD"]["rotation"]["w"] << " \033[0m" << std::endl;
//        std::cout << std::endl;
//        std::cout << " \033[31m Left: " << j3["LeftController"]["position"]["x"] << j3["LeftController"]["position"]["y"] << j3["LeftController"]["position"]["z"] << " \033[0m" << std::endl;
//        std::cout << " \033[31m Left: " << j3["LeftController"]["rotation"]["x"] << j3["LeftController"]["rotation"]["y"] << j3["LeftController"]["rotation"]["z"] << j3["LeftController"]["rotation"]["w"] << " \033[0m" << std::endl;
//        std::cout << std::endl;
//        std::cout << " \033[31m Right: " << j3["RightController"]["position"]["x"] << j3["RightController"]["position"]["y"] << j3["RightController"]["position"]["z"] << " \033[0m" << std::endl;
//        std::cout << " \033[31m Right: " << j3["RightController"]["rotation"]["x"] << j3["RightController"]["rotation"]["y"] << j3["RightController"]["rotation"]["z"] << j3["RightController"]["rotation"]["w"] << " \033[0m" << std::endl;


        //Left Controler
        ControllerButtons[0].ButtonPrimary = j3["LeftController"]["ButtonPrimary"];
        ControllerButtons[0].ButtonSecondary = j3["LeftController"]["ButtonSecondary"];
        //Special
        ControllerButtons[0].ButtonThumbrest = j3["LeftController"]["ButtonThumbrest"];
        ControllerButtons[0].ButtonGrip = j3["LeftController"]["ButtonGrip"];
        ControllerButtons[0].ButtonTrigger = j3["LeftController"]["ButtonTrigger"];
        ControllerButtons[0].ButtonJoystick = j3["LeftController"]["ButtonJoystick"];
        ControllerButtons[0].PressureTrigger = j3["LeftController"]["PressureTrigger"];
        ControllerButtons[0].PressureGrip = j3["LeftController"]["PressureGrip"];
        ControllerButtons[0].JoyStickX = j3["LeftController"]["JoystickPosition"]["x"];
        ControllerButtons[0].JoyStickY = j3["LeftController"]["JoystickPosition"]["y"];

        //Right Controler
        ControllerButtons[1].ButtonPrimary = j3["RightController"]["ButtonPrimary"];
        ControllerButtons[1].ButtonSecondary = j3["RightController"]["ButtonSecondary"];
        //Special
        //Special
        ControllerButtons[1].ButtonThumbrest = j3["RightController"]["ButtonThumbrest"];
        ControllerButtons[1].ButtonGrip = j3["RightController"]["ButtonGrip"];
        ControllerButtons[1].ButtonTrigger = j3["RightController"]["ButtonTrigger"];
        ControllerButtons[1].ButtonJoystick = j3["RightController"]["ButtonJoystick"];
        ControllerButtons[1].PressureTrigger = j3["RightController"]["PressureTrigger"];
        ControllerButtons[1].PressureGrip = j3["RightController"]["PressureGrip"];
        ControllerButtons[1].JoyStickX = j3["RightController"]["JoystickPosition"]["x"];
        ControllerButtons[1].JoyStickY = j3["RightController"]["JoystickPosition"]["y"];

        for (auto &b : ControllerButtons) {
            //print all buttons
            std::cout << " \033[31m ButtonPrimary: " << b.ButtonPrimary << " \033[0m" << std::endl;
            std::cout << " \033[31m ButtonSecondary: " << b.ButtonSecondary << " \033[0m" << std::endl;
            std::cout << " \033[31m ButtonThumbrest: " << b.ButtonThumbrest << " \033[0m" << std::endl;
            std::cout << " \033[31m ButtonGrip: " << b.ButtonGrip << " \033[0m" << std::endl;
            std::cout << " \033[31m ButtonTrigger: " << b.ButtonTrigger << " \033[0m" << std::endl;
            std::cout << " \033[31m ButtonJoystick: " << b.ButtonJoystick << " \033[0m" << std::endl;
            std::cout << " \033[31m PressureTrigger: " << b.PressureTrigger << " \033[0m" << std::endl;
            std::cout << " \033[31m PressureGrip: " << b.PressureGrip << " \033[0m" << std::endl;
            std::cout << " \033[31m JoyStickX: " << b.JoyStickX << " \033[0m" << std::endl;
            std::cout << " \033[31m JoyStickY: " << b.JoyStickY << " \033[0m" << std::endl;
            std::cout << std::endl;
        }


    } catch (std::exception &e) {
        std::cout << " \033[31m UnmarshalJSON: " << e.what() << " \033[0m" << std::endl;
    }

}

std::string CServerDriver_Sample::createJSON(std::map<std::string, std::string> &values) {
    std::string json = "{";
    for (auto &v : values) {
        json += "\"" + v.first + "\":\"" + v.second + "\",";
    }
    json.pop_back();
    json += "}";
    return json;
}