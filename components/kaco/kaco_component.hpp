#pragma once

#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"
#include "kaco_inverter.hpp"
#include <vector>

namespace esphome {
namespace kaco {

enum class KacoBusState {
    IDLE,
    SEND_REQUEST,
    WAIT_RESPONSE
};

class KacoComponent : public Component, public uart::UARTDevice {
public:
    void set_flow_control_pin(GPIOPin *pin) { flow_control_pin_ = pin; }
    void set_update_interval(uint32_t ms) { this->update_interval_ms_ = ms; }
    void set_timeout(uint32_t ms) { this->timeout_ms_ = ms; }

    void register_inverter(KacoInverter *inv, uint8_t address) {
    inv->set_address(address);
    this->inverters_.push_back(inv);
    }

    void setup() override;
    void loop() override;

protected:
    // State machine
    KacoBusState state_{KacoBusState::IDLE};
    int current_inverter_index_{0};

    // Timing
    uint32_t last_poll_{0};
    uint32_t request_timestamp_{0};
    uint32_t update_interval_ms_{60000};
    uint32_t timeout_ms_{10000};

    // Inverters
    std::vector<KacoInverter *> inverters_;

    // RX buffer for frame assembly
    std::vector<uint8_t> rx_frame_buffer_;
    GPIOPin *flow_control_pin_{nullptr};

    // Helpers
    void send_frame_(const std::vector<uint8_t> &frame);
    bool read_frame_(std::vector<uint8_t> &out);
};

}  // namespace kaco
}  // namespace esphome
