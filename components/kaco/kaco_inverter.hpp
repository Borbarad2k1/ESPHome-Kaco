#pragma once

#include "esphome/core/component.h"
#include "esphome/core/gpio.h"
#include "esphome/core/helpers.h"
#include "sensor/kaco_sensor.hpp"
#include "text_sensor/kaco_text_sensor.hpp"
#include <vector>

namespace esphome {
namespace kaco {

class KacoInverter : public Component {
public:
  void set_address(uint8_t address) { this->address_ = address; }
  uint8_t get_address() const { return this->address_; }

  // Called by Python codegen
  void register_sensor(KacoSensor *sensor, const std::string &type) {
    this->sensors_.push_back({type, sensor});
  }

  void register_text_sensor(KacoTextSensor *sensor, const std::string &type) {
    this->text_sensors_.push_back({type, sensor});
  }

  // Called by the bus when a frame for this inverter arrives
  void on_frame(const std::vector<uint8_t> &frame);

  // Called by the bus to request the next poll frame
  std::vector<uint8_t> build_next_request();

protected:
  uint8_t address_{0};

  struct SensorEntry {
    std::string type;
    KacoSensor *sensor;
  };
  std::vector<SensorEntry> sensors_;

  struct TextSensorEntry {
    std::string type;
    KacoTextSensor *sensor;
  };
  std::vector<TextSensorEntry> text_sensors_;
};

}  // namespace kaco
}  // namespace esphome
