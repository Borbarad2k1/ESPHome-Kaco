# ESPHome-Kaco  
**External ESPHome Component for KACO Inverters**

This ESPHome external component allows you to read data from KACO photovoltaic inverters via RS485. It supports multiple inverters on the same bus and exposes their values as ESPHome sensors and text sensors.

---

## 📦 Installation

Add the component to your ESPHome configuration using the `external_components:` block:

```yaml
external_components:
  - source: github://Borbarad2k1/ESPHome-Kaco@main
    components: [ kaco ]
```

ESPHome will automatically download the component during compilation.

---

## 🔌 Hardware Requirements

- ESP32 (recommended)  
- RS485 transceiver (e.g., MAX485)  
- Connection to KACO inverter RS485 terminals  
- GPIO pin for RS485 flow control (DE/RE)

> **Note:**  
> The component uses a dedicated `flow_control_pin` to toggle the RS485 transceiver between transmit and receive.  
> This pin **must not** be configured inside the `uart:` block because:  
> - Only ESP32 supports hardware flow control  
> - Hardware flow control did **not** work reliably with some of my KACO devices  
>  
> Therefore, the component handles flow control manually.

---

## ⚙️ Configuration

Below is a complete example configuration showing how to set up UART, the KACO bus, and sensors.

### UART Setup

```yaml
uart:
  id: kaco_uart
  tx_pin: GPIO23
  rx_pin: GPIO22
  baud_rate: 9600
```

### KACO Bus Setup

```yaml
kaco:
  - id: kaco_bus
    uart_id: kaco_uart
    update_interval: 10s
    flow_control_pin: GPIO18   # IMPORTANT: handled by the component, not by UART!

    inverters:
      - id: inv1
        address: 1
      - id: inv2
        address: 2
      - id: inv3
        address: 3
      - id: inv4
        address: 4
```

### Text Sensors

```yaml
text_sensor:
  - platform: kaco
    inverter_id: inv1
    type: status
    name: "Inverter 1 Status-Text"
  - platform: kaco
    inverter_id: inv2
    type: status
    name: "Inverter 2 Status-Text"
  - platform: kaco
    inverter_id: inv3
    type: status
    name: "Inverter 3 Status-Text"
  - platform: kaco
    inverter_id: inv4
    type: status
    name: "Inverter 4 Status-Text"
```

### Sensors

The component exposes the following sensor types:

| Type | Description |
|------|-------------|
| `status` | Numeric inverter status code |
| `generator_voltage` | DC input voltage |
| `generator_current` | DC input current |
| `generator_power` | DC input power |
| `grid_voltage` | AC output voltage |
| `grid_current` | AC output current |
| `grid_power` | AC output power |
| `temperature` | Internal inverter temperature |
| `daily_yield` | Daily energy yield |

Example configuration:

```yaml
sensor:
  - platform: kaco
    inverter_id: inv1
    type: status
    name: "Inverter 1 Status"

  - platform: kaco
    inverter_id: inv1
    type: generator_voltage
    name: "Inverter 1 Generator Voltage"

  - platform: kaco
    inverter_id: inv1
    type: generator_current
    name: "Inverter 1 Generator Current"

  - platform: kaco
    inverter_id: inv1
    type: generator_power
    name: "Inverter 1 Generator Power"

  - platform: kaco
    inverter_id: inv1
    type: grid_voltage
    name: "Inverter 1 Grid Voltage"

  - platform: kaco
    inverter_id: inv1
    type: grid_current
    name: "Inverter 1 Grid Current"

  - platform: kaco
    inverter_id: inv1
    type: grid_power
    name: "Inverter 1 Grid Power"

  - platform: kaco
    inverter_id: inv1
    type: temperature
    name: "Inverter 1 Temperature"

  - platform: kaco
    inverter_id: inv1
    type: daily_yield
    name: "Inverter 1 Daily Yield"
```

Repeat for additional inverters as needed.

---

## 📝 Notes

- The component polls each inverter sequentially.  
- If an inverter does not respond, ESPHome logs a warning but continues polling others.  
- The RS485 bus must be properly terminated (typically 120 Ω at both ends).  
- All inverters must have unique addresses.

---

## 🧪 Debugging

You can enable UART debugging if needed:

```yaml
uart:
  ...
  debug:
    direction: BOTH
    dummy_receiver: true
    after:
      timeout: 5s
```

This helps diagnose wiring or protocol issues.

---

## 📄 License

MIT License — see repository for details.
