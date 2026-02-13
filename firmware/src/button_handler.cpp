#include "button_handler.h"
#include "config.h"
#include <Arduino.h>
#include <cstring>

/* ============================================================================
 * Button Handler Implementation
 * Debounced GPIO button input with event queueing
 * Non-blocking interrupt-driven design
 * ========================================================================== */

/* Button IDs */
enum ButtonID {
    BUTTON_PREV = 0,
    BUTTON_PLAY = 1,
    BUTTON_NEXT = 2,
    NUM_BUTTONS = 3
};

/* Button event queue (note: avoid EVENT_QUEUE_SIZE, conflicts with config.h macro) */
static const size_t BTN_QUEUE_SIZE = 16;
struct ButtonQueueEvent {
    uint8_t button_id;
    ButtonEvent event;
};

class ButtonHandlerImpl : public ButtonHandler {
private:
    /* Debounce state machine per button */
    struct ButtonState {
        uint8_t pin;
        uint32_t last_change_time;
        ButtonEvent current_state;
        ButtonEvent debounced_state;
    };
    
    ButtonState buttons[NUM_BUTTONS];
    
    /* Event queue */
    ButtonQueueEvent event_queue[BTN_QUEUE_SIZE];
    volatile uint8_t queue_head = 0;
    volatile uint8_t queue_tail = 0;
    volatile size_t queue_count = 0;
    
    static ButtonHandlerImpl* instance;  /* For ISR context */
    
    void enqueue_event(uint8_t button_id, ButtonEvent event);
    void do_debounce();
    
public:
    ButtonHandlerImpl();
    bool init() override;
    bool get_event(uint8_t& button_id, ButtonEvent& event) override;
    size_t pending_count() const override;
    void check_debounce();  /* Called periodically */
    
    friend void button_isr_prev();
    friend void button_isr_play();
    friend void button_isr_next();
};

/* Global singleton */
ButtonHandlerImpl* ButtonHandlerImpl::instance = nullptr;

ButtonHandlerImpl::ButtonHandlerImpl() {
    instance = this;
    
    /* Initialize button state */
    buttons[BUTTON_PREV].pin = BTN_PREV_PIN;
    buttons[BUTTON_PREV].current_state = BUTTON_RELEASED;
    buttons[BUTTON_PREV].debounced_state = BUTTON_RELEASED;
    buttons[BUTTON_PREV].last_change_time = 0;
    
    buttons[BUTTON_PLAY].pin = BTN_PLAY_PIN;
    buttons[BUTTON_PLAY].current_state = BUTTON_RELEASED;
    buttons[BUTTON_PLAY].debounced_state = BUTTON_RELEASED;
    buttons[BUTTON_PLAY].last_change_time = 0;
    
    buttons[BUTTON_NEXT].pin = BTN_NEXT_PIN;
    buttons[BUTTON_NEXT].current_state = BUTTON_RELEASED;
    buttons[BUTTON_NEXT].debounced_state = BUTTON_RELEASED;
    buttons[BUTTON_NEXT].last_change_time = 0;
}

/* ISR handlers without context (Arduino attachInterrupt signature) */
void IRAM_ATTR button_isr_prev() {
    if (ButtonHandlerImpl::instance) {
        ButtonHandlerImpl::instance->check_debounce();
    }
}

void IRAM_ATTR button_isr_play() {
    if (ButtonHandlerImpl::instance) {
        ButtonHandlerImpl::instance->check_debounce();
    }
}

void IRAM_ATTR button_isr_next() {
    if (ButtonHandlerImpl::instance) {
        ButtonHandlerImpl::instance->check_debounce();
    }
}

void ButtonHandlerImpl::enqueue_event(uint8_t button_id, ButtonEvent event) {
    if (queue_count >= BTN_QUEUE_SIZE) {
        Serial.println("[BTN] WARNING: Event queue full");
        return;
    }
    
    event_queue[queue_tail].button_id = button_id;
    event_queue[queue_tail].event = event;
    queue_tail = (queue_tail + 1) % BTN_QUEUE_SIZE;
    queue_count++;
}

void ButtonHandlerImpl::do_debounce() {
    uint32_t now = millis();
    
    for (uint8_t i = 0; i < NUM_BUTTONS; i++) {
        uint8_t raw_state = digitalRead(buttons[i].pin);
        /* Buttons are active-low (pulled up), so invert */
        ButtonEvent raw_event = (raw_state == LOW) ? BUTTON_PRESSED : BUTTON_RELEASED;
        
        /* Debounce FSM */
        if (raw_event != buttons[i].current_state) {
            /* State changed; check if stable for BTN_DEBOUNCE_MS */
            if ((now - buttons[i].last_change_time) >= BTN_DEBOUNCE_MS) {
                buttons[i].current_state = raw_event;
                buttons[i].last_change_time = now;
                
                /* Actual state change confirmed */
                if (buttons[i].current_state != buttons[i].debounced_state) {
                    buttons[i].debounced_state = buttons[i].current_state;
                    enqueue_event(i, buttons[i].debounced_state);
                    
                    const char* btn_names[] = {"PREV", "PLAY", "NEXT"};
                    const char* state_str = (buttons[i].debounced_state == BUTTON_PRESSED) ? "PRESS" : "RELEASE";
                    Serial.printf("[BTN] %s %s\n", btn_names[i], state_str);
                }
            }
        } else {
            buttons[i].last_change_time = now;
        }
    }
}

bool ButtonHandlerImpl::init() {
    Serial.println("[BTN] Initializing button handler");
    
    /* Configure GPIO pins as inputs with pull-ups */
    pinMode(BTN_PREV_PIN, INPUT_PULLUP);
    pinMode(BTN_PLAY_PIN, INPUT_PULLUP);
    pinMode(BTN_NEXT_PIN, INPUT_PULLUP);
    
    /* Attach ISR handlers (Arduino signature: void handler(void)) */
    attachInterrupt(digitalPinToInterrupt(BTN_PREV_PIN), button_isr_prev, CHANGE);
    attachInterrupt(digitalPinToInterrupt(BTN_PLAY_PIN), button_isr_play, CHANGE);
    attachInterrupt(digitalPinToInterrupt(BTN_NEXT_PIN), button_isr_next, CHANGE);
    
    Serial.printf("[BTN] Configured GPIO pins: %d (prev), %d (play), %d (next)\n",
                 BTN_PREV_PIN, BTN_PLAY_PIN, BTN_NEXT_PIN);
    Serial.println("[BTN] Button handler ready");
    
    return true;
}

bool ButtonHandlerImpl::get_event(uint8_t& button_id, ButtonEvent& event) {
    if (queue_count == 0) {
        return false;
    }
    
    button_id = event_queue[queue_head].button_id;
    event = event_queue[queue_head].event;
    queue_head = (queue_head + 1) % BTN_QUEUE_SIZE;
    queue_count--;
    
    return true;
}

size_t ButtonHandlerImpl::pending_count() const {
    return queue_count;
}

void ButtonHandlerImpl::check_debounce() {
    do_debounce();
}

/* Global instance */
static ButtonHandlerImpl g_button_handler;

/* Periodic update function (call from main loop) */
void button_handler_update() {
    g_button_handler.check_debounce();
}

/* Factory function */
ButtonHandler* create_button_handler() {
    return &g_button_handler;
}
