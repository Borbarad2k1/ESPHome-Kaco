from esphome.const import CONF_ID
from esphome.components import sensor

import esphome.codegen as cg
import esphome.config_validation as cv

from esphome.const import (
    DEVICE_CLASS_VOLTAGE,
    DEVICE_CLASS_CURRENT,
    DEVICE_CLASS_POWER,
    DEVICE_CLASS_TEMPERATURE,
    DEVICE_CLASS_ENERGY,
    STATE_CLASS_NONE,
    STATE_CLASS_MEASUREMENT,
    STATE_CLASS_TOTAL_INCREASING,
    UNIT_VOLT,
    UNIT_AMPERE,
    UNIT_WATT,
    UNIT_CELSIUS,
    UNIT_WATT_HOURS,
)

from .. import KacoInverter

CONF_INVERTER_ID = "inverter_id"
CONF_TYPE = "type"

# -----------------------------------------------------------------------------
# SENSOR TYPE DEFINITIONS (metadata)
# -----------------------------------------------------------------------------

SENSOR_TYPES = {
    "status": {
        "name": "Inverter Status",
        "unit": "",
        "icon": "mdi:information-outline",
        "accuracy": 0,
        "device_class": None,
        "state_class": STATE_CLASS_NONE,
    },
    "generator_voltage": {
        "name": "Generator Voltage",
        "unit": UNIT_VOLT,
        "icon": "mdi:flash",
        "accuracy": 1,
        "device_class": DEVICE_CLASS_VOLTAGE,
        "state_class": STATE_CLASS_MEASUREMENT,
    },
    "generator_current": {
        "name": "Generator Current",
        "unit": UNIT_AMPERE,
        "icon": "mdi:current-ac",
        "accuracy": 2,
        "device_class": DEVICE_CLASS_CURRENT,
        "state_class": STATE_CLASS_MEASUREMENT,
    },
    "generator_power": {
        "name": "Generator Power",
        "unit": UNIT_WATT,
        "icon": "mdi:solar-power",
        "accuracy": 0,
        "device_class": DEVICE_CLASS_POWER,
        "state_class": STATE_CLASS_MEASUREMENT,
    },

    "grid_voltage": {
        "name": "Grid Voltage",
        "unit": UNIT_VOLT,
        "icon": "mdi:transmission-tower",
        "accuracy": 1,
        "device_class": DEVICE_CLASS_VOLTAGE,
        "state_class": STATE_CLASS_MEASUREMENT,
    },
    "grid_current": {
        "name": "Grid Current",
        "unit": UNIT_AMPERE,
        "icon": "mdi:current-ac",
        "accuracy": 2,
        "device_class": DEVICE_CLASS_CURRENT,
        "state_class": STATE_CLASS_MEASUREMENT,
    },
    "grid_power": {
        "name": "Grid Power",
        "unit": UNIT_WATT,
        "icon": "mdi:transmission-tower-export",
        "accuracy": 0,
        "device_class": DEVICE_CLASS_POWER,
        "state_class": STATE_CLASS_MEASUREMENT,
    },

    "temperature": {
        "name": "Inverter Temperature",
        "unit": UNIT_CELSIUS,
        "icon": "mdi:thermometer",
        "accuracy": 0,
        "device_class": DEVICE_CLASS_TEMPERATURE,
        "state_class": STATE_CLASS_MEASUREMENT,
    },

    "daily_yield": {
        "name": "Daily Yield",
        "unit": UNIT_WATT_HOURS,
        "icon": "mdi:counter",
        "accuracy": 0,
        "device_class": DEVICE_CLASS_ENERGY,
        "state_class": STATE_CLASS_TOTAL_INCREASING,
    },
}

kaco_ns = cg.esphome_ns.namespace("kaco")
KacoSensor = kaco_ns.class_("KacoSensor", sensor.Sensor)

# -----------------------------------------------------------------------------
# CONFIG SCHEMA
# -----------------------------------------------------------------------------

CONFIG_SCHEMA = (
    sensor.sensor_schema(KacoSensor)
    .extend({
        cv.Required(CONF_INVERTER_ID): cv.use_id(KacoInverter),
        cv.Required(CONF_TYPE): cv.one_of(*SENSOR_TYPES.keys(), lower=True),
    })
)

# -----------------------------------------------------------------------------
# CODE GENERATION
# -----------------------------------------------------------------------------

sensor_ns = cg.esphome_ns.namespace("sensor")
StateClasses = sensor_ns.enum("StateClass")
STATE_CLASSES = {
    "": StateClasses.STATE_CLASS_NONE,
    "measurement": StateClasses.STATE_CLASS_MEASUREMENT,
    "total_increasing": StateClasses.STATE_CLASS_TOTAL_INCREASING,
    "total": StateClasses.STATE_CLASS_TOTAL,
    "measurement_angle": StateClasses.STATE_CLASS_MEASUREMENT_ANGLE,
}

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await sensor.register_sensor(var, config)

    # Attach to inverter
    inverter = await cg.get_variable(config[CONF_INVERTER_ID])
    cg.add(inverter.register_sensor(var, config[CONF_TYPE]))

    # Apply metadata
    meta = SENSOR_TYPES[config[CONF_TYPE]]

    cg.add(var.set_type(config[CONF_TYPE]))
    cg.add(var.set_icon(meta["icon"]))
    cg.add(var.set_unit_of_measurement(meta["unit"]))
    cg.add(var.set_accuracy_decimals(meta["accuracy"]))
    
    if(meta["device_class"] != None): 
        cg.add(var.set_device_class(meta["device_class"]))
    if(meta["state_class"] != None): 
        cg.add(var.set_state_class(STATE_CLASSES.get(meta["state_class"])))
