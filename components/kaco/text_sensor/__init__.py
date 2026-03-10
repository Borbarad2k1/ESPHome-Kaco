from esphome.const import CONF_ID
from esphome.components import text_sensor

import esphome.codegen as cg
import esphome.config_validation as cv

from .. import KacoInverter

CONF_INVERTER_ID = "inverter_id"
CONF_TYPE = "type"

# -----------------------------------------------------------------------------
# SENSOR TYPE DEFINITIONS (metadata)
# -----------------------------------------------------------------------------

SENSOR_TYPES = {
    "status": {
        "name": "Inverter Status",
        "icon": "mdi:information-outline",
    },
}

kaco_ns = cg.esphome_ns.namespace("kaco")
KacoTextSensor = kaco_ns.class_("KacoTextSensor", text_sensor.TextSensor)

# -----------------------------------------------------------------------------
# CONFIG SCHEMA
# -----------------------------------------------------------------------------

CONFIG_SCHEMA = (
    text_sensor.text_sensor_schema(KacoTextSensor)
    .extend({
        cv.Required(CONF_INVERTER_ID): cv.use_id(KacoInverter),
        cv.Required(CONF_TYPE): cv.one_of(*SENSOR_TYPES.keys(), lower=True),
    })
)

# -----------------------------------------------------------------------------
# CODE GENERATION
# -----------------------------------------------------------------------------

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await text_sensor.register_text_sensor(var, config)

    # Attach to inverter
    inverter = await cg.get_variable(config[CONF_INVERTER_ID])
    cg.add(inverter.register_text_sensor(var, config[CONF_TYPE]))

    # Apply metadata
    meta = SENSOR_TYPES[config[CONF_TYPE]]

    cg.add(var.set_icon(meta["icon"]))
