#include "cserverdriver_sample.h"

using namespace vr;

EVRInitError CServerDriver_Sample::Init(vr::IVRDriverContext *pDriverContext)
{
    VR_INIT_SERVER_DRIVER_CONTEXT(pDriverContext);
    InitDriverLog( vr::VRDriverLog() );

    DriverLog("CServerDriver_Sample::Init()\n");
    // Creating a new server
    m_pServer = new UpdServer();

    DriverLog("Starting server thread\n");
    // Starting a new thread for the server
    m_pThread = new std::thread(&UpdServer::run, m_pServer);
    m_pThread->detach(); // Detaching the thread

    DriverLog("CServerDriver_Sample::Init() - Done\n");

    m_pNullHmdLatest = new CSampleDeviceDriver();
    vr::VRServerDriverHost()->TrackedDeviceAdded(m_pNullHmdLatest->GetSerialNumber().c_str(), vr::TrackedDeviceClass_HMD, m_pNullHmdLatest);

    m_pController = new CSampleControllerDriver();
    m_pController->SetControllerIndex(1);
    vr::VRServerDriverHost()->TrackedDeviceAdded(m_pController->GetSerialNumber().c_str(), vr::TrackedDeviceClass_Controller, m_pController);

    m_pController2 = new CSampleControllerDriver();
    m_pController2->SetControllerIndex(2);
    vr::VRServerDriverHost()->TrackedDeviceAdded(m_pController2->GetSerialNumber().c_str(), vr::TrackedDeviceClass_Controller, m_pController2);

    return VRInitError_None;
}

void CServerDriver_Sample::Cleanup()
{
    // Clearing a memory after the server
    delete m_pServer;
    m_pServer = nullptr;
    // Cleating after thread
    delete m_pThread;
    m_pThread = nullptr;

    //CleanupDriverLog();
    delete m_pNullHmdLatest;
    m_pNullHmdLatest = NULL;
    delete m_pController;
    m_pController = NULL;
    delete m_pController2;
    m_pController2 = NULL;
}

void CServerDriver_Sample::RunFrame()
{
    if (m_pNullHmdLatest) {
        m_pNullHmdLatest->RunFrame();
    }
    if (m_pController) {
        m_pController->RunFrame();
    }
    if (m_pController2) {
        m_pController2->RunFrame();
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
