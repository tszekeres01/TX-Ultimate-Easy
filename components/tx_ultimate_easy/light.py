"""Addressable LED output with a temporary touch scan layer."""

import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import light
from esphome.const import CONF_NUM_LEDS, CONF_OUTPUT_ID

from . import tx_ultimate_easy_ns

CONF_STRIP = "strip"
CONF_SCAN_LEDS = "scan_leds"

TouchScanLightOutput = tx_ultimate_easy_ns.class_(
    "TouchScanLightOutput", light.AddressableLight
)

CONFIG_SCHEMA = light.ADDRESSABLE_LIGHT_SCHEMA.extend({
    cv.GenerateID(CONF_OUTPUT_ID): cv.declare_id(TouchScanLightOutput),
    cv.Required(CONF_STRIP): cv.use_id(light.AddressableLight),
    cv.Required(CONF_NUM_LEDS): cv.int_range(min=1, max=32),
    cv.Required(CONF_SCAN_LEDS): cv.int_range(min=1, max=32),
}).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    var = cg.new_Pvariable(
        config[CONF_OUTPUT_ID], config[CONF_NUM_LEDS], config[CONF_SCAN_LEDS]
    )
    strip = await cg.get_variable(config[CONF_STRIP])
    cg.add(var.set_strip(strip))
    await cg.register_component(var, config)
    await light.register_light(var, config)
