#include "mm_sound.h"
#include "win_policy.h"
#include <initguid.h> // For PKEY_Device
#include <endpointvolume.h>
#include <functiondiscoverykeys_devpkey.h>
#include <QDebug>

MmSound::MmSound(MmState *st, QObject *parent)
    : QObject(parent)
{
    state = st;
    HRESULT hr = ::CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr,
                                    CLSCTX_INPROC_SERVER,
                                    IID_PPV_ARGS(&device_enum));
    if( hr )
    {
        qDebug() << "CoCreateInstance MMDeviceEnumerator failed"
                 << hr;
        return;
    }

    hr = device_enum->EnumAudioEndpoints(
                eRender, DEVICE_STATE_ACTIVE, &collection);

    if( hr )
    {
        qDebug() << "IMMDeviceCollection::EnumAudioEndpoints: " << hr;
        return;
    }

    mic_timer = new QTimer();
    connect(mic_timer, SIGNAL(timeout()), this, SLOT(micTimeOut()));
    mic_timer->start(15000);
    updateMic();
}

MmSound::~MmSound()
{
    ;
}

void MmSound::micTimeOut()
{
    updateMic();
    int vol = getVolume(mic_dev);
    if( vol<100 )
    {
        qDebug() << "micTimeOut: set volume from" << vol
                 << "to" << 100;
        setVolume(mic_dev, 100);
    }
}

void MmSound::leftClick()
{
    LPWSTR next_iid = NULL;
    int    next_index = getNextIndex();
    qDebug() << "getNextIndex:" << next_index;

    if( next_index<0 )
    {
        qDebug() << "getNextIndex failed:" << next_index;
        return;
    }

    IMMDevice *next_device;
    HRESULT hr = collection->Item(next_index, &next_device);
    if( hr )
    {
        qDebug() << next_index << "Getting Item Failed";
        return;
    }
    next_device->GetId(&next_iid);

    setDevice(next_iid);
}

QString MmSound::getLabel()
{
    QString label;
    IMMDevice *spkr_dev;
    HRESULT hr = device_enum->GetDefaultAudioEndpoint(
                              eRender, eMultimedia, &spkr_dev);
    if( hr )
    {
        qDebug() << "GetDefaultAudioEndpoint Failed" << hr;
        return 0;
    }

    label  = "%{U#815DB4}%{+U}";
    label += "%{A1:sound:}%{A2:right:}%{A4:vol_up:}%{A5:vol_down:}  ";

    if( isHeadset(spkr_dev) )
    {
        label += "\uf025";
    }
    else
    {
        label += "\uf6a8";
    }

    label += " ";
    label += QString::number(getVolume(spkr_dev));

    label += "% %{A5}%{A4}%{A2}%{A1}%{-U}  ";

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

void MmSound::setDevice(LPWSTR dev_iid)
{
    IPolicyConfigVista *pPolicyConfig;
    HRESULT hr = CoCreateInstance(CLSID_CPolicyConfigVistaClient,
                 NULL, CLSCTX_ALL, IID_IPolicyConfigVista,
                 (LPVOID *)&pPolicyConfig);

    if( hr )
    {
        qDebug() << "CoCreateInstance CPolicyConfigVistaClient Failed";
        return;
    }

    hr = pPolicyConfig->SetDefaultEndpoint(dev_iid, eConsole);
    pPolicyConfig->Release();
}

int MmSound::getNextIndex()
{
    IMMDevice *default_dev;
    LPWSTR     default_iid = NULL;
    LPWSTR     current_iid = NULL;

    HRESULT hr = device_enum->GetDefaultAudioEndpoint(
                              eRender, eMultimedia, &default_dev);
    if( hr )
    {
        qDebug() << "GetDefaultAudioEndpoint Failed" << hr;
        return -1;
    }
    default_dev->GetId(&default_iid);

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
            return -1;
        }
        p_device->GetId(&current_iid);

        if( wcscmp(default_iid, current_iid)==0 )
        {
            if( i+1<count )
            {
                return i+1;
            }
            else
            {
                return 0;
            }
        }

//        qDebug() << "[" << i << "]\t" << getName(p_device);
    }

    return -1;
}

void MmSound::updateMic()
{
    HRESULT hr;

    if( state->mic_name.length()==0 )
    {
        updateDefualtMic();
    }

    // Retrieve the number of active audio devices for the specified direction
    UINT count = 0;
    collection->GetCount(&count);

    for( UINT i=0 ; i<count ; i++ )
    {
        IMMDevice *p_device;
        hr = collection->Item(i, &p_device);
        if( hr )
        {
            qDebug() << i << "Getting Item Failed";
            return;
        }
//        qDebug() << "[" << i << "]\t" << getName(p_device);
        QString mic_dev_name = getName(p_device);
        if( mic_dev_name==state->mic_name )
        {
            mic_dev = p_device;
            return;
        }
    }
    updateDefualtMic();
}

void MmSound::updateDefualtMic()
{
    HRESULT hr = device_enum->GetDefaultAudioEndpoint(
                              eCapture, eMultimedia, &mic_dev);
    if( hr )
    {
        qDebug() << "updateDefualtMic Failed" << hr;
        return;
    }
}

void MmSound::volumeUp()
{
    IMMDevice *spkr_dev;
    HRESULT hr = device_enum->GetDefaultAudioEndpoint(
                              eRender, eMultimedia, &spkr_dev);
    if( hr )
    {
        qDebug() << "GetDefaultAudioEndpoint Failed" << hr;
        return;
    }

    IAudioEndpointVolume *endpointVolume = NULL;
    hr = spkr_dev->Activate(__uuidof(IAudioEndpointVolume),
                              CLSCTX_INPROC_SERVER, NULL, (LPVOID *)&endpointVolume);

    float volume = 0;
    hr = endpointVolume->GetMasterVolumeLevelScalar(&volume);

    if( volume<0.9 )
    {
        volume += 0.1;
    }
    else
    {
        volume = 1;
    }

    endpointVolume->SetMasterVolumeLevelScalar(volume, NULL);

    endpointVolume->Release();
}

void MmSound::volumeDown()
{
    IMMDevice *spkr_dev;
    HRESULT hr = device_enum->GetDefaultAudioEndpoint(
                              eRender, eMultimedia, &spkr_dev);
    if( hr )
    {
        qDebug() << "GetDefaultAudioEndpoint Failed" << hr;
        return;
    }

    IAudioEndpointVolume *endpointVolume = NULL;
    hr = spkr_dev->Activate(__uuidof(IAudioEndpointVolume),
                    CLSCTX_INPROC_SERVER, NULL, (LPVOID *)&endpointVolume);

    float volume = 0;
    hr = endpointVolume->GetMasterVolumeLevelScalar(&volume);

    if( volume>0.1 )
    {
        volume -= 0.1;
    }
    else
    {
        volume = 0;
    }

    endpointVolume->SetMasterVolumeLevelScalar(volume, NULL);

    endpointVolume->Release();
}

void MmSound::setVolume(IMMDevice *spkr_dev, int volume)
{
    float volume_f = volume/100.0;
    IAudioEndpointVolume *endpointVolume = NULL;
    spkr_dev->Activate(__uuidof(IAudioEndpointVolume),
                    CLSCTX_INPROC_SERVER, NULL, (LPVOID *)&endpointVolume);
    endpointVolume->SetMasterVolumeLevelScalar(volume_f, NULL);
    endpointVolume->Release();
}
