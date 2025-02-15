#pragma once

#include <map>
#include <napi.h>
#include <string>
#include <vector>

#define VALID_VOLUME_BALANCE(balance)             \
    ((balance.right < 1.F && balance.right > 0.F) \
        && (balance.left < 1.F && balance.left > 0.F))

#define EVENT_CHANGE_MASK_MUTE 1
#define EVENT_CHANGE_MASK_VOLUME 2 * EVENT_CHANGE_MASK_MUTE
#define EVENT_CHANGE_MASK_CHANNEL_COUNT 2 * EVENT_CHANGE_MASK_VOLUME

namespace SoundMixerUtils
{
typedef struct
{
    int flags;
    float volume;
    bool mute;
} NotificationHandler;

void CallJs(Napi::Env env, Napi::Function cb,
    Napi::Reference<Napi::Value> *context, NotificationHandler *data);

using TSFN = Napi::TypedThreadSafeFunction<Napi::Reference<Napi::Value>,
    NotificationHandler, CallJs>;

enum DeviceType
{
    OUTPUT = 0,
    INPUT = 1,
    ALL = 2
};

enum EventType
{
    VOLUME = 0,
    MUTE = 1,
    COUNT = 2
};

typedef struct
{
    std::string fullName;
    std::string id;
    DeviceType type;
} DeviceDescriptor;
typedef struct
{
    std::string id;
    std::string fullName;
} AudioSessionDescriptor;
bool deviceEquals(DeviceDescriptor a, DeviceDescriptor b);
uint32_t hashcode(DeviceDescriptor device);

typedef struct
{
    float right;
    float left;
    bool stereo;
} VolumeBalance;

class EventPool {
  public:
    EventPool();
    virtual ~EventPool();
    // Event handlers for devices
    int RegisterEvent(DeviceDescriptor device, EventType type, TSFN value);
    bool RemoveEvent(DeviceDescriptor device, EventType type, int id);
    std::vector<TSFN> GetListeners(DeviceDescriptor dev, EventType type);
    void RemoveAllListeners(DeviceDescriptor device, EventType type);
    // Event handlers for audio sessions
    int RegisterEvent(
        AudioSessionDescriptor session, EventType type, TSFN value);
    bool RemoveEvent(AudioSessionDescriptor session, EventType type, int id);
    std::vector<TSFN> GetListeners(
        AudioSessionDescriptor session, EventType type);
    void RemoveAllListeners(AudioSessionDescriptor session, EventType type);
    void Clear();

  private:
    static uint32_t getHashCode(std::string, EventType type);

  private:
    std::map<uint32_t, std::map<int, TSFN>> m_events;
    int counter = 0;
};
} // namespace SoundMixerUtils
