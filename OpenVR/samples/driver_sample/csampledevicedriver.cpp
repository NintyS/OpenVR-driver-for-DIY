#include "csampledevicedriver.h"

#include "basics.h"

#include <math.h>
#include "driverlog.h"

using namespace vr;


CSampleDeviceDriver::CSampleDeviceDriver()
{
    std::cout << "\033[31m CSampleDeviceDriver::CSampleDeviceDriver() Start \033[0m" << std::endl;

    m_unObjectId = vr::k_unTrackedDeviceIndexInvalid;
    m_ulPropertyContainer = vr::k_ulInvalidPropertyContainer;

    //DriverLog( "Using settings values\n" );
    m_flIPD = vr::VRSettings()->GetFloat(k_pch_SteamVR_Section, k_pch_SteamVR_IPD_Float);

    char buf[1024];
    vr::VRSettings()->GetString(k_pch_Sample_Section, k_pch_Sample_SerialNumber_String, buf, sizeof(buf));
    m_sSerialNumber = buf;

    vr::VRSettings()->GetString(k_pch_Sample_Section, k_pch_Sample_ModelNumber_String, buf, sizeof(buf));
    m_sModelNumber = buf;

//    m_nWindowX = vr::VRSettings()->GetInt32(k_pch_Sample_Section, k_pch_Sample_WindowX_Int32);
//    m_nWindowY = vr::VRSettings()->GetInt32(k_pch_Sample_Section, k_pch_Sample_WindowY_Int32);
//    m_nWindowWidth = vr::VRSettings()->GetInt32(k_pch_Sample_Section, k_pch_Sample_WindowWidth_Int32);
//    m_nWindowHeight = vr::VRSettings()->GetInt32(k_pch_Sample_Section, k_pch_Sample_WindowHeight_Int32);
//    m_nRenderWidth = vr::VRSettings()->GetInt32(k_pch_Sample_Section, k_pch_Sample_RenderWidth_Int32);
//    m_nRenderHeight = vr::VRSettings()->GetInt32(k_pch_Sample_Section, k_pch_Sample_RenderHeight_Int32);
    m_flSecondsFromVsyncToPhotons = vr::VRSettings()->GetFloat(k_pch_Sample_Section, k_pch_Sample_SecondsFromVsyncToPhotons_Float);
//    m_flDisplayFrequency = vr::VRSettings()->GetFloat(k_pch_Sample_Section, k_pch_Sample_DisplayFrequency_Float);

    m_nWindowX = 0;
    m_nWindowY = 0;
    m_nWindowWidth = 1920;
    m_nWindowHeight = 1080;
    m_nRenderWidth = 1920;
    m_nRenderHeight = 1080;

    m_flDisplayFrequency = 120;

    /*DriverLog( "driver_null: Serial Number: %s\n", m_sSerialNumber.c_str() );
        DriverLog( "driver_null: Model Number: %s\n", m_sModelNumber.c_str() );
        DriverLog( "driver_null: Window: %d %d %d %d\n", m_nWindowX, m_nWindowY, m_nWindowWidth, m_nWindowHeight );
        DriverLog( "driver_null: Render Target: %d %d\n", m_nRenderWidth, m_nRenderHeight );
        DriverLog( "driver_null: Seconds from Vsync to Photons: %f\n", m_flSecondsFromVsyncToPhotons );
        DriverLog( "driver_null: Display Frequency: %f\n", m_flDisplayFrequency );
        DriverLog( "driver_null: IPD: %f\n", m_flIPD );*/

    std::cout << "\033[31m CSampleDeviceDriver::CSampleDeviceDriver() End \033[0m" << std::endl;
}

CSampleDeviceDriver::~CSampleDeviceDriver()
{
    delete m_thread;
    delete m_threadImageChecking;
//    delete m_threadDisplaying;
}

EVRInitError CSampleDeviceDriver::Activate(TrackedDeviceIndex_t unObjectId)
{
    std::cout << "\033[31m CSampleDeviceDriver::Activate() Start \033[0m" << std::endl;

    m_unObjectId = unObjectId;
    m_ulPropertyContainer = vr::VRProperties()->TrackedDeviceToPropertyContainer(m_unObjectId);

    vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_ModelNumber_String, m_sModelNumber.c_str());
    vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_RenderModelName_String, m_sModelNumber.c_str());
    vr::VRProperties()->SetFloatProperty(m_ulPropertyContainer, Prop_UserIpdMeters_Float, m_flIPD);
    vr::VRProperties()->SetFloatProperty(m_ulPropertyContainer, Prop_UserHeadToEyeDepthMeters_Float, 0.f);
    vr::VRProperties()->SetFloatProperty(m_ulPropertyContainer, Prop_DisplayFrequency_Float, m_flDisplayFrequency);
    vr::VRProperties()->SetFloatProperty(m_ulPropertyContainer, Prop_SecondsFromVsyncToPhotons_Float, m_flSecondsFromVsyncToPhotons);

    // return a constant that's not 0 (invalid) or 1 (reserved for Oculus)
    vr::VRProperties()->SetUint64Property(m_ulPropertyContainer, Prop_CurrentUniverseId_Uint64, 1);

    // avoid "not fullscreen" warnings from vrmonitor
    vr::VRProperties()->SetBoolProperty(m_ulPropertyContainer, Prop_IsOnDesktop_Bool, false);

    //Debug mode activate Windowed Mode (borderless fullscreen), locked to 30 FPS, for testing
    vr::VRProperties()->SetBoolProperty(m_ulPropertyContainer, Prop_DisplayDebugMode_Bool, true);

    // Icons can be configured in code or automatically configured by an external file "drivername\resources\driver.vrresources".
    // Icon properties NOT configured in code (post Activate) are then auto-configured by the optional presence of a driver's "drivername\resources\driver.vrresources".
    // In this manner a driver can configure their icons in a flexible data driven fashion by using an external file.
    //
    // The structure of the driver.vrresources file allows a driver to specialize their icons based on their HW.
    // Keys matching the value in "Prop_ModelNumber_String" are considered first, since the driver may have model specific icons.
    // An absence of a matching "Prop_ModelNumber_String" then considers the ETrackedDeviceClass ("HMD", "Controller", "GenericTracker", "TrackingReference")
    // since the driver may have specialized icons based on those device class names.
    //
    // An absence of either then falls back to the "system.vrresources" where generic device class icons are then supplied.
    //
    // Please refer to "bin\drivers\sample\resources\driver.vrresources" which contains this sample configuration.
    //
    // "Alias" is a reserved key and specifies chaining to another json block.
    //
    // In this sample configuration file (overly complex FOR EXAMPLE PURPOSES ONLY)....
    //
    // "Model-v2.0" chains through the alias to "Model-v1.0" which chains through the alias to "Model-v Defaults".
    //
    // Keys NOT found in "Model-v2.0" would then chase through the "Alias" to be resolved in "Model-v1.0" and either resolve their or continue through the alias.
    // Thus "Prop_NamedIconPathDeviceAlertLow_String" in each model's block represent a specialization specific for that "model".
    // Keys in "Model-v Defaults" are an example of mapping to the same states, and here all map to "Prop_NamedIconPathDeviceOff_String".
    //
    bool bSetupIconUsingExternalResourceFile = true;
    if (!bSetupIconUsingExternalResourceFile) {
        // Setup properties directly in code.
        // Path values are of the form {drivername}\icons\some_icon_filename.png
        vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_NamedIconPathDeviceOff_String, "{sample}/icons/quest_headset_off@2x.png");
        vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_NamedIconPathDeviceSearching_String, "{sample}/icons/quest_headset_searching@2x.b4bfb144.gif");
        vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_NamedIconPathDeviceSearchingAlert_String, "{sample}/icons/quest_headset_alert_searching@2x.b4bfb144.gif");
        vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_NamedIconPathDeviceReady_String, "{sample}/icons/quest_headset_ready@2x.b4bfb144.png");
        vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_NamedIconPathDeviceReadyAlert_String, "{sample}/icons/quest_headset_ready_alert@2x.b4bfb144.png");
        vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_NamedIconPathDeviceNotReady_String, "{sample}/icons/quest_headset_error.b4bfb144.png");
        vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_NamedIconPathDeviceStandby_String, "{sample}/icons/quest_headset_standby@2x.b4bfb144.png");
        vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_NamedIconPathDeviceAlertLow_String, "{sample}/icons/headset_sample_status_ready_low.png");
    }

    m_thread = new std::thread(&CSampleDeviceDriver::PositionUpdater, this);
    m_threadImageChecking = new std::thread(&CSampleDeviceDriver::GetImage, this);
//    m_threadDisplaying = new std::thread(&CSampleDeviceDriver::DisplayImage, this);
    m_bExiting = true;

    m_thread->detach();
    m_threadImageChecking->detach();
//    m_threadDisplaying->detach();

    std::cout << "\033[31m CSampleDeviceDriver::Activate() done \033[0m" << std::endl;

    return VRInitError_None;
}

void CSampleDeviceDriver::Deactivate()
{
    if(m_bExiting.exchange(false)) {
        m_thread->join();
        m_threadImageChecking->join();
//        m_threadDisplaying->join();
    }

    m_unObjectId = vr::k_unTrackedDeviceIndexInvalid;
}

void CSampleDeviceDriver::EnterStandby()
{
}

void *CSampleDeviceDriver::GetComponent(const char *pchComponentNameAndVersion)
{
    if (strcmp(pchComponentNameAndVersion, vr::IVRDisplayComponent_Version) == 0)
    {
        return static_cast<vr::IVRDisplayComponent *>(this);
    }

//    if (strcmp(pchComponentNameAndVersion, vr::IVRVirtualDisplay_Version) == 0)
//    {
//        return static_cast<vr::IVRVirtualDisplay *>(this);
//    }

    // Obsługa innych interfejsów

    return nullptr;
}

void CSampleDeviceDriver::PowerOff()
{

}

void CSampleDeviceDriver::DebugRequest(const char *pchRequest, char *pchResponseBuffer, uint32_t unResponseBufferSize)
{
    if (unResponseBufferSize >= 1) {
        pchResponseBuffer[0] = 0;
    }
}

void CSampleDeviceDriver::GetWindowBounds(int32_t *pnX, int32_t *pnY, uint32_t *pnWidth, uint32_t *pnHeight)
{
    *pnX = m_nWindowX;
    *pnY = m_nWindowY;
    *pnWidth = m_nWindowWidth;
    *pnHeight = m_nWindowHeight;
}

bool CSampleDeviceDriver::IsDisplayOnDesktop()
{
    return false;
}

bool CSampleDeviceDriver::IsDisplayRealDisplay()
{
    return false;
}

void CSampleDeviceDriver::GetRecommendedRenderTargetSize(uint32_t *pnWidth, uint32_t *pnHeight)
{
    *pnWidth = m_nRenderWidth;
    *pnHeight = m_nRenderHeight;
}

void CSampleDeviceDriver::GetEyeOutputViewport(EVREye eEye, uint32_t *pnX, uint32_t *pnY, uint32_t *pnWidth, uint32_t *pnHeight)
{
    *pnY = 0;
    *pnWidth = m_nWindowWidth / 2;
    *pnHeight = m_nWindowHeight;

    if (eEye == Eye_Left) {
        *pnX = 0;
    } else {
        *pnX = m_nWindowWidth / 2;
    }
}

void CSampleDeviceDriver::GetProjectionRaw(EVREye eEye, float *pfLeft, float *pfRight, float *pfTop, float *pfBottom)
{
    *pfLeft = -1.0;
    *pfRight = 1.0;
    *pfTop = -1.0;
    *pfBottom = 1.0;
}

DistortionCoordinates_t CSampleDeviceDriver::ComputeDistortion(EVREye eEye, float fU, float fV)
{
    DistortionCoordinates_t coordinates;
    coordinates.rfBlue[0] = fU;
    coordinates.rfBlue[1] = fV;
    coordinates.rfGreen[0] = fU;
    coordinates.rfGreen[1] = fV;
    coordinates.rfRed[0] = fU;
    coordinates.rfRed[1] = fV;
    return coordinates;
}

vr::DriverPose_t CSampleDeviceDriver::GetPose() // This function sets postion of controlers, headsets and other shit
{
    vr::DriverPose_t pose = { 0 };
    pose.poseIsValid = true;
    pose.result = vr::TrackingResult_Running_OK;
    pose.deviceIsConnected = true;

    pose.qWorldFromDriverRotation = HmdQuaternion_Init(1, 0, 0, 0);
    pose.qDriverFromHeadRotation = HmdQuaternion_Init(1, 0, 0, 0);

    pose.vecPosition[0] = HMDPosition.x;
    pose.vecPosition[1] = HMDPosition.y;
    pose.vecPosition[2] = -HMDPosition.z;

    pose.qRotation.w = HMDPosition.kata;
    pose.qRotation.x = -HMDPosition.yaw;
    pose.qRotation.y = -HMDPosition.pitch;
    pose.qRotation.z = HMDPosition.roll;

    return pose;
}

void CSampleDeviceDriver::Present( const vr::PresentInfo_t *pPresentInfo, uint32_t unPresentInfoSize )
{
    // red text in srd::cout
    std::cout << "\033[31m Present output: " << pPresentInfo->backbufferTextureHandle << " \033[0m" << std::endl;

}

void CSampleDeviceDriver::WaitForPresent()
{

}

bool CSampleDeviceDriver::GetTimeSinceLastVsync(float *pfSecondsSinceLastVsync, uint64_t *pulFrameCounter)
{
    return true;
}

void CSampleDeviceDriver::PositionUpdater()
{
    while(m_bExiting) {
        if (m_unObjectId != vr::k_unTrackedDeviceIndexInvalid) {
            vr::VRServerDriverHost()->TrackedDevicePoseUpdated(m_unObjectId, GetPose(), sizeof(DriverPose_t));
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }

}

void CSampleDeviceDriver::RunFrame()
{
//    std::cout << "\033[31m CSampleDeviceDriver::RunFrame() \033[0m" << std::endl;

    // In a real driver, this should happen from some pose tracking thread.
    // The RunFrame interval is unspecified and can be very irregular if some other
    // driver blocks it for some periodic task.
//    if (m_unObjectId != vr::k_unTrackedDeviceIndexInvalid) {
//        vr::VRServerDriverHost()->TrackedDevicePoseUpdated(m_unObjectId, GetPose(), sizeof(DriverPose_t));
//    }
}

void CSampleDeviceDriver::SetPosition(Point3D &position)
{
    HMDPosition = position;
}

void CSampleDeviceDriver::GetImage() {
    Display* display = XOpenDisplay(nullptr);
    Window rootWindow = DefaultRootWindow(display);
    XWindowAttributes attributes;
    XGetWindowAttributes(display, rootWindow, &attributes);
    cv::Mat mat(attributes.height,
                attributes.width,
                CV_8UC3);
    while(m_bExiting) {
        XImage *image = XGetImage(display,
                                  rootWindow,
                                  0,
                                  0,
                                  attributes.width,
                                  attributes.height,
                                  AllPlanes,
                                  ZPixmap);
        cv::Mat raw_mat(image->height,
                        image->width,
                        CV_8UC4,
                        image->data,
                        image->bytes_per_line);
        cv::cvtColor(raw_mat, mat, cv::COLOR_RGBA2RGB);
        XDestroyImage(image);
        image = nullptr;
        cv::Rect region(mat.cols / 2, 0, mat.cols / 2, mat.rows);
        cv::Mat cropped = mat(region);
        if(cropped.empty()) {
            std::cout << "cropped is empty" << std::endl;
        } else {
            std::cout << "cropped is not empty" << std::endl;
            if(m_matImage.data == cropped.data) {
                std::cout << "Equal" << std::endl;
                raw_mat.release();
                cropped.release();
                mat.release();
                continue;
            } else {
                std::cout << "NewFrame" << std::endl;
                m_matImage = cropped.clone();
                raw_mat.release();
                cropped.release();
                mat.release();
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    mat.release();
    rootWindow = 0;
    XCloseDisplay(display);
    display = nullptr;
}

cv::Mat CSampleDeviceDriver::GetEyeView() {
    if(m_matImage.empty()) {
        std::cout << "m_matImage is empty" << std::endl;
        return {};
    }

    return m_matImage;
}