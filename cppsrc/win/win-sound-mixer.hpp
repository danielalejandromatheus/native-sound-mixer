#pragma once

#include <Audioclient.h>
#include <audiopolicy.h>
#include <endpointvolume.h>
#include <mmdeviceapi.h>
#include <string>
#include <vector>
#include "sound-mixer-utils.hpp"

#define LEFT 0
#define RIGHT 1

using SoundMixerUtils::AudioSessionDescriptor;
using SoundMixerUtils::DeviceDescriptor;
using SoundMixerUtils::DeviceType;
using SoundMixerUtils::NotificationHandler;
using SoundMixerUtils::VolumeBalance;

namespace WinSoundMixer
{

typedef void (*on_device_changed_cb_t)(
    DeviceDescriptor dev, NotificationHandler);
typedef void (*on_session_changed_cb_t)(
    AudioSessionDescriptor sess, NotificationHandler);
class AudioSession {
  public:
    AudioSession(IAudioSessionControl2 *control);
    AudioSession(IAudioSessionControl2 *control, on_session_changed_cb_t cb);
    ~AudioSession();

    virtual bool GetMute();
    virtual void SetMute(bool);
    virtual float GetVolume();
    virtual void SetVolume(float);

    virtual void SetVolumeBalance(const VolumeBalance &);
    virtual VolumeBalance GetVolumeBalance();

    std::string id();
    int pid();
    std::string name();
    std::string path();
    AudioSessionState state();

    float _oldVolume = 0.F;
    BOOL _oldMute = 0.F;
    AudioSessionDescriptor Desc()
    {
        return desc;
    }
    on_session_changed_cb_t _sessionCallback;
  protected:
    IAudioSessionControl2 *control;
    ISimpleAudioVolume *getAudioVolume();
    IAudioSessionEvents *audio_cb;
    AudioSessionDescriptor desc;
};

class Device {
  public:
    Device(IMMDevice *, on_device_changed_cb_t cb, on_session_changed_cb_t s_cb);
    ~Device();

    virtual bool GetMute();
    virtual void SetMute(bool);
    virtual float GetVolume();
    virtual void SetVolume(float);

    virtual void SetVolumeBalance(const VolumeBalance &);
    virtual VolumeBalance GetVolumeBalance();

    /**
     *  \brief      Updates the info of the Device.
     */
    virtual bool Update();

    /**
     *  \brief      Returns whether the device is still available.
     */
    bool IsValid();

    DeviceDescriptor Desc()
    {
        return desc;
    }

    std::vector<AudioSession *> GetAudioSessions();
    AudioSession *GetAudioSessionById(std::string);

    static IMMDeviceEnumerator *GetEnumerator();
    on_device_changed_cb_t _deviceCallback;
    on_session_changed_cb_t _sessionCallback;

    float _oldVolume = 0.F;
    BOOL _oldMute = 0.F;

  protected:
    IMMDevice *device;
    IMMEndpoint *endpoint;
    DeviceDescriptor desc;
    IAudioEndpointVolume *endpointVolume;
    IAudioEndpointVolumeCallback *device_cb;

  private:
    bool valid = true;
};

class SoundMixer {
  public:
    SoundMixer(on_device_changed_cb_t, on_session_changed_cb_t);
    ~SoundMixer();
    std::vector<Device *> GetDevices();
    Device *GetDefaultDevice(DeviceType);

  private:
    IMMDeviceEnumerator *pEnumerator = nullptr;
    std::map<std::string, Device *> devices;

  private:
    /**
     *  \brief  Returns the corresponding device if already in the list,
     *  NULL otherwise.
     *
     *  \fn     Device *getDeviceById(LPWSTR id);
     *  \param id   The id of the device to find.
     *  \returns    The corresponding device if already in the list, NULL
     *  otherwise.
     *  \remarks    The caller is responsible for freeing the provided id.
     */
    Device *getDeviceById(LPWSTR id);

    void filterDevices();
    on_device_changed_cb_t deviceCallback;
    on_session_changed_cb_t sessionCallback;
};
class SoundMixerAudioEndpointVolumeCallback
    : public IAudioEndpointVolumeCallback {
  public:
    SoundMixerAudioEndpointVolumeCallback(Device *dev);

    IFACEMETHODIMP_(ULONG) AddRef();
    IFACEMETHODIMP_(ULONG) Release();

  private:
    IFACEMETHODIMP OnNotify(PAUDIO_VOLUME_NOTIFICATION_DATA pNotify);
    IFACEMETHODIMP QueryInterface(const IID &iid, void **ppUnk);

  private:
    Device *device;
};

class SoundMixerAudioSessionEvent : public IAudioSessionEvents {
  public:
    SoundMixerAudioSessionEvent(AudioSession *session);

    IFACEMETHODIMP_(ULONG) AddRef();
    IFACEMETHODIMP_(ULONG) Release();

  private:
    IFACEMETHODIMP OnDisplayNameChanged(
        LPCWSTR NewDisplayName, LPCGUID EventContext);
    IFACEMETHODIMP OnIconPathChanged(
        LPCWSTR NewIconPath, LPCGUID EventContext);
    IFACEMETHODIMP OnSimpleVolumeChanged(
        float NewVolume, BOOL NewMute, LPCGUID EventContext);
    IFACEMETHODIMP OnChannelVolumeChanged(DWORD ChannelCount,
        float NewChannelVolumeArray[], DWORD ChangedChannel,
        LPCGUID EventContext);
    IFACEMETHODIMP OnGroupingParamChanged(
        LPCGUID NewGroupingParam, LPCGUID EventContext);
    IFACEMETHODIMP OnStateChanged(AudioSessionState NewState);
    IFACEMETHODIMP OnSessionDisconnected(
        AudioSessionDisconnectReason DisconnectReason);
    IFACEMETHODIMP QueryInterface(const IID &iid, void **ppUnk);

  protected:
    AudioSession *session;
};
}; // namespace WinSoundMixer
