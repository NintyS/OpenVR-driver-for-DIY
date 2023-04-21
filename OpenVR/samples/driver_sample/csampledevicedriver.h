#ifndef CSAMPLEDEVICEDRIVER_H
#define CSAMPLEDEVICEDRIVER_H

#include <openvr_driver.h>
#include <iostream>
#include "struct.h"
#include <atomic>
#include <thread>

#include <opencv2/opencv.hpp>

#include <X11/Xutil.h>
#include <X11/Xlib.h>

#undef None
#undef Status
#undef Response
#undef BadRequest

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
class CSampleDeviceDriver : public vr::ITrackedDeviceServerDriver, public vr::IVRDisplayComponent, public vr::IVRVirtualDisplay
{
public:
    CSampleDeviceDriver();

    virtual ~CSampleDeviceDriver();

    virtual vr::EVRInitError Activate(vr::TrackedDeviceIndex_t unObjectId);

    virtual void Deactivate();

    virtual  void EnterStandby();

    void *GetComponent(const char *pchComponentNameAndVersion);

    virtual void PowerOff();

    /** debug request from a client */
    virtual void DebugRequest(const char *pchRequest, char *pchResponseBuffer, uint32_t unResponseBufferSize);

    virtual void GetWindowBounds(int32_t *pnX, int32_t *pnY, uint32_t *pnWidth, uint32_t *pnHeight);

    // Setting visibility of image

    virtual bool IsDisplayOnDesktop();

    virtual bool IsDisplayRealDisplay();

    // Setting image

    virtual void GetRecommendedRenderTargetSize(uint32_t *pnWidth, uint32_t *pnHeight);

    virtual void GetEyeOutputViewport(vr::EVREye eEye, uint32_t *pnX, uint32_t *pnY, uint32_t *pnWidth, uint32_t *pnHeight);

    virtual void GetProjectionRaw(vr::EVREye eEye, float *pfLeft, float *pfRight, float *pfTop, float *pfBottom);

    virtual vr::DistortionCoordinates_t ComputeDistortion(vr::EVREye eEye, float fU, float fV);

    // Getting image
    virtual void Present( const vr::PresentInfo_t *pPresentInfo, uint32_t unPresentInfoSize );

    virtual void WaitForPresent();

    virtual bool GetTimeSinceLastVsync( float *pfSecondsSinceLastVsync, uint64_t *pulFrameCounter );

    // Getting & setting pose

    virtual vr::DriverPose_t GetPose();

    void SetPosition(Point3D &position);

    void PositionUpdater();

    void RunFrame();

    std::string GetSerialNumber() const { return m_sSerialNumber; }

    cv::Mat GetEyeView();

    void GetImage();
//
//    void DisplayImage();
private:
    vr::TrackedDeviceIndex_t m_unObjectId;
    vr::PropertyContainerHandle_t m_ulPropertyContainer;

    std::string m_sSerialNumber;
    std::string m_sModelNumber;

    cv::Mat m_matImage;

    Point3D HMDPosition;

    std::thread *m_thread;
    std::thread *m_threadImageChecking;
//    std::thread *m_threadDisplaying;
    std::atomic<bool> m_bExiting = true;

    int32_t m_nWindowX;
    int32_t m_nWindowY;
    int32_t m_nWindowWidth;
    int32_t m_nWindowHeight;
    int32_t m_nRenderWidth;
    int32_t m_nRenderHeight;
    float m_flSecondsFromVsyncToPhotons;
    float m_flDisplayFrequency;
    float m_flIPD;
};

#endif // CSAMPLEDEVICEDRIVER_H
