#ifndef WIN_POLICY_H
#define WIN_POLICY_H

#include <Windows.h>
#include <mmdeviceapi.h>

interface DECLSPEC_UUID("f8679f50-850a-41cf-9c72-430f290290c8")
IPolicyConfig;
class DECLSPEC_UUID("870af99c-171d-4f9e-af0d-e63df40c2bc9")
CPolicyConfigClient;
// ----------------------------------------------------------------------------
// class CPolicyConfigClient
// {870af99c-171d-4f9e-af0d-e63df40c2bc9}
//
// interface IPolicyConfig
// {f8679f50-850a-41cf-9c72-430f290290c8}
//
// Query interface:
// CComPtr<IPolicyConfig> PolicyConfig;
// PolicyConfig.CoCreateInstance(__uuidof(CPolicyConfigClient));
//
// @compatible: Windows 7 and Later
// ----------------------------------------------------------------------------
interface IPolicyConfig : public IUnknown
{
public:

        virtual HRESULT GetMixFormat(
                PCWSTR,
                WAVEFORMATEX **
        );

        virtual HRESULT STDMETHODCALLTYPE GetDeviceFormat(
                PCWSTR,
                INT,
                WAVEFORMATEX **
        );

        virtual HRESULT STDMETHODCALLTYPE ResetDeviceFormat(
                PCWSTR
        );

        virtual HRESULT STDMETHODCALLTYPE SetDeviceFormat(
                PCWSTR,
                WAVEFORMATEX *,
                WAVEFORMATEX *
        );

        virtual HRESULT STDMETHODCALLTYPE GetProcessingPeriod(
                PCWSTR,
                INT,
                PINT64,
                PINT64
        );

        virtual HRESULT STDMETHODCALLTYPE SetProcessingPeriod(
                PCWSTR,
                PINT64
        );

        virtual HRESULT STDMETHODCALLTYPE GetShareMode(
                PCWSTR,
                struct DeviceShareMode *
        );

        virtual HRESULT STDMETHODCALLTYPE SetShareMode(
                PCWSTR,
                struct DeviceShareMode *
        );

        virtual HRESULT STDMETHODCALLTYPE GetPropertyValue(
                PCWSTR,
                const PROPERTYKEY &,
                PROPVARIANT *
        );

        virtual HRESULT STDMETHODCALLTYPE SetPropertyValue(
                PCWSTR,
                const PROPERTYKEY &,
                PROPVARIANT *
        );

        virtual HRESULT STDMETHODCALLTYPE SetDefaultEndpoint(
                PCWSTR wszDeviceId,
                ERole eRole
        );

        virtual HRESULT STDMETHODCALLTYPE SetEndpointVisibility(
                PCWSTR,
                INT
        );
};

interface DECLSPEC_UUID("568b9108-44bf-40b4-9006-86afe5b5a620")
IPolicyConfigVista;
class DECLSPEC_UUID("294935CE-F637-4E7C-A41B-AB255460B862")
CPolicyConfigVistaClient;

const CLSID CLSID_CPolicyConfigVistaClient = {
    0x294935CE, 0xF637, 0x4E7C, {0xA4, 0x1B, 0xAB, 0x25, 0x54, 0x60, 0xB8, 0x62} };

const IID IID_IPolicyConfigVista = {
    0x568b9108, 0x44bf, 0x40b4, {0x90, 0x06, 0x86, 0xaf, 0xe5, 0xb5, 0xa6, 0x20} };
// ----------------------------------------------------------------------------
// class CPolicyConfigVistaClient
// {294935CE-F637-4E7C-A41B-AB255460B862}
//
// interface IPolicyConfigVista
// {568b9108-44bf-40b4-9006-86afe5b5a620}
//
// Query interface:
// CComPtr<IPolicyConfigVista> PolicyConfig;
// PolicyConfig.CoCreateInstance(__uuidof(CPolicyConfigVistaClient));
//
// @compatible: Windows Vista and Later
// ----------------------------------------------------------------------------
interface IPolicyConfigVista : public IUnknown
{
public:

        virtual HRESULT GetMixFormat(
                PCWSTR,
                WAVEFORMATEX **
        );  // not available on Windows 7, use method from IPolicyConfig

        virtual HRESULT STDMETHODCALLTYPE GetDeviceFormat(
                PCWSTR,
                INT,
                WAVEFORMATEX **
        );

        virtual HRESULT STDMETHODCALLTYPE SetDeviceFormat(
                PCWSTR,
                WAVEFORMATEX *,
                WAVEFORMATEX *
        );

        virtual HRESULT STDMETHODCALLTYPE GetProcessingPeriod(
                PCWSTR,
                INT,
                PINT64,
                PINT64
        );  // not available on Windows 7, use method from IPolicyConfig

        virtual HRESULT STDMETHODCALLTYPE SetProcessingPeriod(
                PCWSTR,
                PINT64
        );  // not available on Windows 7, use method from IPolicyConfig

        virtual HRESULT STDMETHODCALLTYPE GetShareMode(
                PCWSTR,
                struct DeviceShareMode *
        );  // not available on Windows 7, use method from IPolicyConfig

        virtual HRESULT STDMETHODCALLTYPE SetShareMode(
                PCWSTR,
                struct DeviceShareMode *
        );  // not available on Windows 7, use method from IPolicyConfig

        virtual HRESULT STDMETHODCALLTYPE GetPropertyValue(
                PCWSTR,
                const PROPERTYKEY &,
                PROPVARIANT *
        );

        virtual HRESULT STDMETHODCALLTYPE SetPropertyValue(
                PCWSTR,
                const PROPERTYKEY &,
                PROPVARIANT *
        );

        virtual HRESULT STDMETHODCALLTYPE SetDefaultEndpoint(
                PCWSTR wszDeviceId,
                ERole eRole
        );

        virtual HRESULT STDMETHODCALLTYPE SetEndpointVisibility(
                PCWSTR,
                INT
        );  // not available on Windows 7, use method from IPolicyConfig
};

#endif // WIN_POLICY_H
