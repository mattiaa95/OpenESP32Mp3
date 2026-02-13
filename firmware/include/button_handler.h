#ifndef BUTTON_HANDLER_H
#define BUTTON_HANDLER_H

#include <cstdint>
#include <cstddef>

/* ============================================================================
 * Button Handler Interface (Pure Virtual)
 * Debounced button input with event queueing
 * ========================================================================== */

enum ButtonEvent {
    BUTTON_RELEASED = 0,
    BUTTON_PRESSED = 1,
};

class ButtonHandler {
public:
    virtual ~ButtonHandler() = default;
    
    /* Initialize GPIO pins and ISRs */
    virtual bool init() = 0;
    
    /* Get next button event (non-blocking) */
    virtual bool get_event(uint8_t& button_id, ButtonEvent& event) = 0;
    
    /* Check pending event count */
    virtual size_t pending_count() const = 0;
};

#endif  // BUTTON_HANDLER_H
