#include "kaco_component.hpp"
#include "esphome/core/log.h"

namespace esphome {
namespace kaco {

static const char *const TAG = "kaco.bus";

void KacoComponent::setup()
{
    if (this->flow_control_pin_ != nullptr)
    {
        this->flow_control_pin_->setup();  // OUTPUT
        this->flow_control_pin_->digital_write(false);
    }
    ESP_LOGI(TAG, "KACO bus initialized");
    this->state_ = KacoBusState::IDLE;
}

void KacoComponent::loop()
{
    uint32_t now = millis();

    switch (this->state_)
    {
        case KacoBusState::IDLE:
        {
            // Overflow-safe diff
            uint32_t diff = (uint32_t)(now - this->last_poll_);
            // Number of inverters (at least 1)
            uint32_t div = this->inverters_.empty() ? 1 : this->inverters_.size();
            // Per-inverter interval
            uint32_t interval = this->update_interval_ms_ / div;
            if (diff < interval)
            {
                return;
            }
            this->last_poll_ = now;

            if (this->inverters_.empty())
                return;

            this->current_inverter_index_++;
            if (this->current_inverter_index_ >= (int)this->inverters_.size())
                this->current_inverter_index_ = 0;

            this->state_ = KacoBusState::SEND_REQUEST;
            break;
        }

        case KacoBusState::SEND_REQUEST:
        {
            auto *inv = this->inverters_[this->current_inverter_index_];
            auto req = inv->build_next_request();

            if (req.empty())
            {
                ESP_LOGW(TAG, "Inverter %d returned empty request", inv->get_address());
                this->state_ = KacoBusState::IDLE;
                return;
            }

            // Reset RX buffer before sending
            this->rx_frame_buffer_.clear();

            ESP_LOGW(TAG, "Sending request to inverter %d", inv->get_address());
            this->send_frame_(req);
            
            this->request_timestamp_ = now;
            this->state_ = KacoBusState::WAIT_RESPONSE;
            break;
        }

        case KacoBusState::WAIT_RESPONSE:
        {
            if (this->read_frame_(this->rx_frame_buffer_))
            {
                ESP_LOGD(TAG, "Got frame.");
                auto *inv = this->inverters_[this->current_inverter_index_];
                inv->on_frame(this->rx_frame_buffer_);

                this->rx_frame_buffer_.clear();
                this->state_ = KacoBusState::IDLE;
                return;
            }

            if (now - this->request_timestamp_ > this->timeout_ms_)
            {
                ESP_LOGW(TAG, "Timeout waiting for inverter %d",
                    this->inverters_[this->current_inverter_index_]->get_address());

                this->rx_frame_buffer_.clear();
                this->state_ = KacoBusState::IDLE;
            }
            break;
        }
    }
}


void KacoComponent::send_frame_(const std::vector<uint8_t> &frame)
{
    this->flow_control_pin_->digital_write(true);
    this->write_array(frame);
    this->flush();
    this->flow_control_pin_->digital_write(false);

    ESP_LOGV(TAG, "TX: %s", format_hex_pretty(frame).c_str());
}

bool KacoComponent::read_frame_(std::vector<uint8_t> &out)
{
    uint8_t byte;

    while (this->available())
    {
        if(!this->read_byte(&byte))
        {
            return false;
        }

        // If we have no frame started yet, drop until '\n'
        if (out.empty())
        {
            if (byte == '\n')
            {
                out.push_back(byte);  // start frame
            }
            // else: ignore garbage
            continue;
        }

        // We are inside a frame
        out.push_back(byte);

        // End of frame?
        if (byte == '\r')
        {
            return true;  // complete frame
        }
    }
    return false;  // no complete frame yet
}


}  // namespace kaco
}  // namespace esphome
