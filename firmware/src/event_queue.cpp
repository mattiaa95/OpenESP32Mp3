#include "event_queue.h"
#include "config.h"
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

/* ============================================================================
 * Event Queue Implementation
 * FreeRTOS-based thread-safe queue for inter-task communication
 * ========================================================================== */

class EventQueueImpl : public EventQueue {
private:
    QueueHandle_t xQueue;
    
public:
    EventQueueImpl();
    ~EventQueueImpl();
    
    bool post(const Event& event) override;
    bool wait_and_receive(Event& event, uint32_t timeout_ms) override;
    bool try_receive(Event& event) override;
    size_t pending_count() const override;
};

EventQueueImpl::EventQueueImpl() {
    /* Create FreeRTOS queue: 20 event capacity */
    xQueue = xQueueCreate(20, sizeof(Event));
    if (xQueue == nullptr) {
        Serial.println("[EVT] FATAL: Could not create event queue");
    }
}

EventQueueImpl::~EventQueueImpl() {
    if (xQueue) {
        vQueueDelete(xQueue);
    }
}

bool EventQueueImpl::post(const Event& event) {
    if (!xQueue) return false;
    
    /* Non-blocking post from interrupt or task context */
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    BaseType_t result = xQueueSendToBackFromISR(xQueue, &event, &xHigherPriorityTaskWoken);
    
    if (result == pdFAIL) {
        Serial.println("[EVT] WARNING: Event queue full");
        return false;
    }
    
    if (xHigherPriorityTaskWoken) {
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
    
    return true;
}

bool EventQueueImpl::wait_and_receive(Event& event, uint32_t timeout_ms) {
    if (!xQueue) return false;
    
    TickType_t timeout_ticks = (timeout_ms == 0) ? portMAX_DELAY : (timeout_ms / portTICK_PERIOD_MS);
    BaseType_t result = xQueueReceive(xQueue, &event, timeout_ticks);
    
    return (result == pdTRUE);
}

bool EventQueueImpl::try_receive(Event& event) {
    if (!xQueue) return false;
    
    BaseType_t result = xQueueReceive(xQueue, &event, 0);  /* No timeout */
    return (result == pdTRUE);
}

size_t EventQueueImpl::pending_count() const {
    if (!xQueue) return 0;
    return uxQueueMessagesWaiting(xQueue);
}

/* Global singleton */
static EventQueueImpl g_event_queue;

EventQueue* create_event_queue() {
    return &g_event_queue;
}
