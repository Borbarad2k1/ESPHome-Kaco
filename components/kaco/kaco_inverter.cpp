#include "kaco_inverter.hpp"
#include "esphome/core/log.h"

#include <map>

namespace esphome {
namespace kaco {

static const char *const TAG = "kaco.inverter";

static std::string status_to_str(int status)
{
    switch (status)
    {
        case 0:  return "STARTUP";
        case 1:  return "WAIT_START";
        case 2:  return "WAIT_SHUTDOWN";
        case 3:  return "CV_REGULATOR";
        case 4:  return "MPP_SEARCH";
        case 5:  return "MPP_FIXED";
        case 6:  return "WAIT_GRIDFEED";
        case 7:  return "WAIT_SELFTEST";
        case 8:  return "SELFTEST_RELAY";

        case 10: return "SHUTDOWN_OVERTEMP";
        case 11: return "LIMIT_POWER";
        case 12: return "SHUTDOWN_OVERLOAD";
        case 13: return "SHUTDOWN_OVERVOLTAGE";
        case 14: return "SHUTDOWN_GRID";
        case 15: return "SHUTDOWN_NIGHT";

        case 18: return "SHUTDOWN_RCD_B";
        case 19: return "SHUTDOWN_INSULATION";

        case 30: return "FAULT_VOLT_TRANSFORMER";
        case 31: return "FAULT_RCD_B_MODULE";
        case 32: return "FAULT_SELFTEST";
        case 33: return "FAULT_DC_FEED";
        case 34: return "FAULT_COMMUNICATION";

        default: return "UNKNOWN";
    }
}

void KacoInverter::on_frame(const std::vector<uint8_t> &frame)
{
    // LF + 64 chars + CR
    if (frame.size() < 66)
    {
        ESP_LOGW(TAG, "Frame too short (%d bytes)", frame.size());
        return;
    }

    // Strip LF and CR
    std::string line(frame.begin() + 1, frame.end() - 1);

    auto get_field = [&](int start, int len) -> std::string {
        if (start < 0 || start + len > (int) line.size())
            return "";
        return line.substr(start, len);
    };

    auto trim = [](std::string s) -> std::string {
        auto p = s.find_first_not_of(' ');
        if (p == std::string::npos) return "";
        auto q = s.find_last_not_of(' ');
        return s.substr(p, q - p + 1);
    };

    auto to_float = [&](const std::string &s) -> float {
        std::string t = trim(s);
        if (t.empty()) return NAN;
        return atof(t.c_str());
    };

    auto to_int = [&](const std::string &s) -> int {
        std::string t = trim(s);
        if (t.empty()) return 0;
        return atoi(t.c_str());
    };

    // Fields (0-based in `line`)
    std::string status_str = get_field(5, 3);
    std::string vdc_str    = get_field(9, 5);
    std::string idc_str    = get_field(15, 5);
    std::string pdc_str    = get_field(21, 5);
    std::string vac_str    = get_field(27, 5);
    std::string iac_str    = get_field(33, 5);
    std::string pac_str    = get_field(39, 5);
    std::string temp_str   = get_field(45, 3);
    std::string yield_str  = get_field(49, 6);
    char checksum_field    = line[56];
    std::string wr_type    = get_field(58, 6);  // "5000xi" in your example

    // Optional: checksum (sum from '*' up to and including space after E)
    uint8_t calc = 0;
    bool started = false;
    for (size_t i = 0; i < frame.size(); i++)
    {
        uint8_t b = frame[i];
        if (!started)
        {
            if (b == '*') started = true;
            else continue;
        }
        calc += b;
        // space after E is at line[55] → frame index 1+55 = 56
        if (i == 56) break;
    }
    if (calc != static_cast<uint8_t>(checksum_field))
    {
        ESP_LOGW(TAG, "Checksum mismatch: frame=%u calc=%u",
            (uint8_t) checksum_field, calc);
        // decide if you want to return here or just log
    }

    // Convert values
    int   status = to_int(status_str);
    float vdc    = to_float(vdc_str);
    float idc    = to_float(idc_str);
    float pdc    = to_float(pdc_str);
    float vac    = to_float(vac_str);
    float iac    = to_float(iac_str);
    float pac    = to_float(pac_str);
    float temp   = to_float(temp_str);
    float daily  = to_float(yield_str);
    std::string status_text = status_to_str(status);

    // Build numeric value map (status handled separately)
    std::map<std::string, float> values;
    values["generator_voltage"] = vdc;
    values["generator_current"] = idc;
    values["generator_power"]   = pdc;
    values["grid_voltage"]      = vac;
    values["grid_current"]      = iac;
    values["grid_power"]        = pac;
    values["temperature"]       = temp;
    values["daily_yield"]       = daily;

    // Publish to sensors
    for (auto &entry : this->sensors_)
    {
        if (!entry.sensor) continue;

        if (entry.type == "status")
        {
            entry.sensor->publish_state(status);
            continue;
        }

        auto it = values.find(entry.type);
        if (it != values.end())
            entry.sensor->publish_state(it->second);
    }
    for (auto &entry : this->text_sensors_)
    {
        if (!entry.sensor) continue;

        if (entry.type == "status")
        {
            entry.sensor->publish_state(status_text);
            continue;
        }
    }

    ESP_LOGD(TAG,
        "Inv %d: S=%d Vdc=%.1f Idc=%.2f Pdc=%.0f Vac=%.1f Iac=%.2f Pac=%.0f T=%.0f E=%.0f",
        this->address_, status, vdc, idc, pdc, vac, iac, pac, temp, daily);
}


std::vector<uint8_t> KacoInverter::build_next_request()
{
    std::vector<uint8_t> frame;

    frame.push_back('\n');          // Start
    frame.push_back('#');           // Prefix

    // Address as 2 ASCII digits
    char addr_hi = '0' + (this->address_ / 10);
    char addr_lo = '0' + (this->address_ % 10);
    frame.push_back(addr_hi);
    frame.push_back(addr_lo);

    frame.push_back('0');           // Command code (poll)
    frame.push_back('\r');          // End

    return frame;
}


}  // namespace kaco
}  // namespace esphome
