#include "button_handler.h"

class ButtonHandlerImpl : public ButtonHandler {
private:
    
public:
    bool init() override;
    bool get_event(uint8_t& button_id, ButtonEvent& event) override;
    size_t pending_count() const override;
};

bool ButtonHandlerImpl::init() {
    // TODO: Initialize GPIO ISRs and debounce task
    return false;
}

bool ButtonHandlerImpl::get_event(uint8_t& button_id, ButtonEvent& event) {
    // TODO: Get next button event from queue
    return false;
}

size_t ButtonHandlerImpl::pending_count() const {
    // TODO: Return pending event count
    return 0;
}
