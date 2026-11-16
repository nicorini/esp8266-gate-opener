// HomeKit accessory definition. The library's macros use C99 compound literals, so this part
// stays in C; main.cpp sets the pairing code and the setter.
//
// The gate is a momentary switch, not a garage door: it has no state to show, and HomeKit asks
// for confirmation on an iPhone before Siri on a HomePod opens a garage door, but not a switch.

#include <homekit/characteristics.h>
#include <homekit/homekit.h>

homekit_characteristic_t cha_switch_on = HOMEKIT_CHARACTERISTIC_(ON, false);

static void identify(homekit_value_t value) { (void)value; }

homekit_accessory_t* accessories[] = {
    HOMEKIT_ACCESSORY(
            .id = 1, .category = homekit_accessory_category_switch,
            .services =
                (homekit_service_t*[]){
                    HOMEKIT_SERVICE(
                        ACCESSORY_INFORMATION,
                        .characteristics =
                            (homekit_characteristic_t*[]){
                                HOMEKIT_CHARACTERISTIC(NAME, "Gate Opener"),
                                HOMEKIT_CHARACTERISTIC(MANUFACTURER, "esp8266-gate-opener"),
                                HOMEKIT_CHARACTERISTIC(SERIAL_NUMBER, "1"),
                                HOMEKIT_CHARACTERISTIC(MODEL, "ESP8266"),
                                HOMEKIT_CHARACTERISTIC(FIRMWARE_REVISION, FIRMWARE_VERSION),
                                HOMEKIT_CHARACTERISTIC(IDENTIFY, identify), NULL}),
                    HOMEKIT_SERVICE(SWITCH, .primary = true,
                                    .characteristics =
                                        (homekit_characteristic_t*[]){
                                            &cha_switch_on,
                                            HOMEKIT_CHARACTERISTIC(NAME, "Gate"), NULL}),
                    NULL}),
    NULL};

homekit_server_config_t config = {.accessories = accessories};
