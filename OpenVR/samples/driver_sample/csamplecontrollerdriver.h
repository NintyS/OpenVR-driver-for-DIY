#ifndef CSAMPLECONTROLLERDRIVER_H
#define CSAMPLECONTROLLERDRIVER_H

#include <openvr_driver.h>
#include <iostream>
#include "struct.h"
#include <thread>
#include <atomic>

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------

class CSampleControllerDriver : public vr::ITrackedDeviceServerDriver
{
    int32_t ControllerIndex;
public:
    CSampleControllerDriver();

    virtual void SetControllerIndex(int32_t CtrlIndex);

    virtual ~CSampleControllerDriver();

    virtual vr::EVRInitError Activate(vr::TrackedDeviceIndex_t unObjectId);

    virtual void Deactivate();

    virtual void EnterStandby();

    void *GetComponent(const char *pchComponentNameAndVersion);

    virtual void PowerOff();

    /** debug request from a client */
    virtual void DebugRequest(const char *pchRequest, char *pchResponseBuffer, uint32_t unResponseBufferSize);

    virtual vr::DriverPose_t GetPose();

    void PositionUpdater();

    void ButtonsUpdater();

    void SetPositions(Point3D &controller, Point3D &hmd);

    void SetButtons(Buttons &buttons);

    void RunFrame();

    void ProcessEvent(const vr::VREvent_t &vrEvent);

    std::string GetSerialNumber() const;

private:
    vr::TrackedDeviceIndex_t m_unObjectId;
    vr::PropertyContainerHandle_t m_ulPropertyContainer;

    //vr::VRInputComponentHandle_t m_compA;
    //vr::VRInputComponentHandle_t m_compB;
    //vr::VRInputComponentHandle_t m_compC;
    vr::VRInputComponentHandle_t m_compHaptic;

    vr::VRInputComponentHandle_t HButtons[7], HAnalog[3];
    //std::string m_sSerialNumber;
    //std::string m_sModelNumber;

    Point3D ControllerPosition;
    Point3D HMDPosition;

    Buttons ControllerButtons;

    std::thread *m_pPositionUpdaterThread;
    std::thread *m_pButtonsUpdaterThread;
    std::atomic<bool> m_bShowController;

};

#endif // CSAMPLECONTROLLERDRIVER_H
