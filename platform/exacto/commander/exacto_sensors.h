#ifndef EXACTO_SENSORS_H
#define EXACTO_SENSORS_H
#include "spi/spi1_generated.h"
#include <stdint.h>
typedef enum {
    LSM303AH = 0,
    ISM330DLC,
    BMP280
}exacto_sensors_list_t;
extern void enableExactoSensor(exacto_sensors_list_t sensor);
extern void disableExactoSensor(exacto_sensors_list_t sensor);
#endif //EXACTO_SENSORS_H