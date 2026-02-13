#ifndef BLUETOOTH_A2DP_H
#define BLUETOOTH_A2DP_H

#include <cstdint>

/* ============================================================================
 * Bluetooth A2DP Source Interface (Pure Virtual)
 * Manages Bluetooth pairing and audio streaming to external speaker
 * ========================================================================== */

class BluetoothA2DP {
public:
    virtual ~BluetoothA2DP() = default;
    
    /* Initialize Bluetooth stack; discover and pair with speaker */
    virtual bool init() = 0;
    
    /* Connect to last bonded device (or broadcast) */
    virtual bool connect() = 0;
    
    /* Disconnect gracefully */
    virtual bool disconnect() = 0;
    
    /* Feed PCM samples to Bluetooth encoder (SBC) */
    virtual bool feed_audio(const int16_t* pcm, uint16_t sample_count) = 0;
    
    /* Check connection status */
    virtual bool is_connected() const = 0;
    
    /* Set volume (0–100) */
    virtual bool set_volume(uint8_t volume) = 0;
    
    /* Error status */
    virtual const char* get_error_message() const = 0;
};

#endif  // BLUETOOTH_A2DP_H
