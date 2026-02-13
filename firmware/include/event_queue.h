#ifndef EVENT_QUEUE_H
#define EVENT_QUEUE_H

#include <cstdint>
#include <cstddef>

/* ============================================================================
 * Event Queue Interface
 * Thread-safe queue for inter-task communication
 * ========================================================================== */

enum EventType {
    EVENT_BUTTON_PREV = 0x10,
    EVENT_BUTTON_PLAY = 0x11,
    EVENT_BUTTON_NEXT = 0x12,
    
    EVENT_PLAYBACK_PLAY = 0x20,
    EVENT_PLAYBACK_PAUSE = 0x21,
    EVENT_PLAYBACK_NEXT = 0x22,
    EVENT_PLAYBACK_PREV = 0x23,
    
    EVENT_AUDIO_BUFFER_READY = 0x30,
    EVENT_AUDIO_UNDERRUN = 0x31,
    EVENT_AUDIO_ERROR = 0x32,
    
    EVENT_BT_CONNECTED = 0x40,
    EVENT_BT_DISCONNECTED = 0x41,
    EVENT_BT_ERROR = 0x42,
    
    EVENT_DISPLAY_REDRAW = 0x50,
    EVENT_DISPLAY_STATE_CHANGE = 0x51,
    
    EVENT_SD_FILE_LOADED = 0x60,
    EVENT_SD_FILE_NOT_FOUND = 0x61,
    EVENT_SD_ERROR = 0x62,
};

typedef struct {
    uint8_t type;
    uint32_t param;
} Event;

class EventQueue {
public:
    virtual ~EventQueue() = default;
    
    virtual bool post(const Event& event) = 0;
    virtual bool wait_and_receive(Event& event, uint32_t timeout_ms) = 0;
    virtual bool try_receive(Event& event) = 0;
    virtual size_t pending_count() const = 0;
};

#endif  // EVENT_QUEUE_H
