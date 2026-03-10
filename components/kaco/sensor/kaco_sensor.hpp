#pragma once

#include "esphome/components/sensor/sensor.h"
#include <string>

namespace esphome {
namespace kaco {

class KacoSensor : public sensor::Sensor {
public:
    void set_type(const std::string &type) { this->type_ = type; }
    const std::string &get_type() const { return this->type_; }

protected:
    std::string type_;
};

}  // namespace kaco
}  // namespace esphome
