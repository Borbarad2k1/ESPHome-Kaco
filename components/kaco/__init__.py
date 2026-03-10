from esphome.components import uart
from esphome import pins
from esphome.const import CONF_ID, CONF_UART_ID, CONF_FLOW_CONTROL_PIN

import esphome.codegen as cg
import esphome.config_validation as cv

DEPENDENCIES = ["uart"]
MULTI_CONF = True

CONF_UPDATE_INTERVAL = "update_interval"
CONF_INVERTERS = "inverters"
CONF_ADDRESS = "address"

kaco_ns = cg.esphome_ns.namespace("kaco")

# Main bus component
KacoComponent = kaco_ns.class_("KacoComponent", cg.Component)

# Inverter object (not a component)
KacoInverter = kaco_ns.class_("KacoInverter")

# -----------------------------------------------------------------------------
# INVERTER SUB-SCHEMA
# -----------------------------------------------------------------------------

INVERTER_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(KacoInverter),
    cv.Required(CONF_ADDRESS): cv.int_,
})

# -----------------------------------------------------------------------------
# MAIN CONFIG SCHEMA
# -----------------------------------------------------------------------------

CONFIG_SCHEMA = (
    cv.Schema({
        cv.GenerateID(): cv.declare_id(KacoComponent),

        cv.Required(CONF_UART_ID): cv.use_id(uart.UARTComponent),

        cv.Required("flow_control_pin"): pins.gpio_output_pin_schema,

        cv.Optional(CONF_UPDATE_INTERVAL, default="60s"): cv.All(
            cv.positive_time_period_milliseconds,
            cv.Range(min=cv.TimePeriod(seconds=10))
        ),

        cv.Required(CONF_INVERTERS): cv.ensure_list(INVERTER_SCHEMA),
    })
    .extend(cv.COMPONENT_SCHEMA)
    .extend(uart.UART_DEVICE_SCHEMA)
)

# -----------------------------------------------------------------------------
# FINAL VALIDATION: UNIQUE ADDRESSES
# -----------------------------------------------------------------------------

def final_validate(config):
    seen = set()
    for inv in config.get(CONF_INVERTERS, []):
        addr = inv[CONF_ADDRESS]
        if addr in seen:
            raise cv.Invalid(f"Duplicate inverter address {addr}")
        seen.add(addr)
    return config

FINAL_VALIDATE_SCHEMA = final_validate

# -----------------------------------------------------------------------------
# CODE GENERATION
# -----------------------------------------------------------------------------

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    # Register UART
    await uart.register_uart_device(var, config)

    # Create the GPIO pin
    flow_pin = await cg.gpio_pin_expression(config[CONF_FLOW_CONTROL_PIN])
    cg.add(var.set_flow_control_pin(flow_pin))

    # Set update interval
    cg.add(var.set_update_interval(config[CONF_UPDATE_INTERVAL]))

    # Create inverter objects
    for inv_conf in config[CONF_INVERTERS]:
        inv = cg.new_Pvariable(inv_conf[CONF_ID])
        cg.add(var.register_inverter(inv, inv_conf[CONF_ADDRESS]))
