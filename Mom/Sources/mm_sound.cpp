#include "mm_sound.h"
#include <initguid.h> // For PKEY_Device
#include <endpointvolume.h>
#include <functiondiscoverykeys_devpkey.h>
#include <QDebug>

MmSound::MmSound(QObject *parent) : QObject(parent)
{
    HRESULT hr = ::CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr,
                                    CLSCTX_INPROC_SERVER,
                                    IID_PPV_ARGS(&device_enum));
    if ( hr==CO_E_NOTINITIALIZED )
    {
        qDebug() << "CoCreateInstance MMDeviceEnumerator failed";
        return;
    }
}

MmSound::~MmSound()
{
    ;
}

void MmSound::leftClick()
{
    IMMDeviceCollection *collection;
    HRESULT hr = device_enum->EnumAudioEndpoints(
                eRender, DEVICE_STATE_ACTIVE, &collection);

    if ( hr )
    {
        qDebug() << "IMMDeviceCollection::EnumAudioEndpoints: " << hr;
        return;
    }
    // Retrieve the number of active audio devices for the specified direction
    UINT count = 0;
    collection->GetCount(&count);

    for ( UINT i=0 ; i<count ; i++ )
    {
        IMMDevice *p_device;
        hr = collection->Item(i, &p_device);
        if( hr )
        {
            qDebug() << i << "Getting Item Failed";
            return ;
        }

        qDebug() << "[" << i << "]\t" << getName(p_device);
    }
}

QString MmSound::getLabel()
{
    QString label;
    IMMDevice *spkr_dev;
    HRESULT hr = device_enum->GetDefaultAudioEndpoint(
                              eRender, eMultimedia, &spkr_dev);
    if ( hr )
    {
        qDebug() << "GetDefaultAudioEndpoint Failed" << hr;
        return 0;
    }

    label  = "%{b#d00}    ";

    if( isHeadset(spkr_dev) )
    {
        label += "\uf025";
    }
    else
    {
        label += "\uf6a8";
    }

    label += "    %{b-}%{a1:sound:}     ";
    label += QString::number(getVolume(spkr_dev));

    label += "    %{a}";

    return label;
}

int MmSound::isHeadset(IMMDevice *dev)
{
    QString dev_name = getName(dev);
    dev_name = dev_name.toLower();
    if( dev_name.contains("headset") )
    {
        return 1;
    }

    return 0;
}

QString MmSound::getName(IMMDevice *dev)
{
    IPropertyStore *p_property_store;
    HRESULT hr = dev->OpenPropertyStore(STGM_READ, &p_property_store);
    if( hr )
    {
        qDebug() << "OpenPropertyStore Failed";
        return "";
    }

    PROPVARIANT p_val;
    PropVariantInit(&p_val);

    hr = p_property_store->GetValue(PKEY_Device_FriendlyName, &p_val);
    if( hr )
    {
        qDebug() << "Getting Value PKEY_Device_FriendlyName Failed";
    }

    QString dev_name = QString::fromStdWString(p_val.pwszVal);

    PropVariantClear(&p_val);

    return dev_name;
}

int MmSound::getVolume(IMMDevice *dev)
{
    IAudioEndpointVolume *endpointVolume = NULL;
    HRESULT hr = dev->Activate(__uuidof(IAudioEndpointVolume),
                              CLSCTX_INPROC_SERVER, NULL, (LPVOID *)&endpointVolume);

    float volume = 0;
    hr = endpointVolume->GetMasterVolumeLevelScalar(&volume);

    endpointVolume->Release();

    return volume*100;
}

void MmSound::setDevice(QString name)
{
    IPolicyConfigVista *pPolicyConfig;
    ERole reserved = eConsole;

    HRESULT hr = CoCreateInstance(__uuidof(CPolicyConfigVistaClient),
        NULL, CLSCTX_ALL, __uuidof(IPolicyConfigVista), (LPVOID *)&pPolicyConfig);
    if (SUCCEEDED(hr))
    {
        hr = pPolicyConfig->SetDefaultEndpoint(devID, reserved);
        pPolicyConfig->Release();
    }
}
