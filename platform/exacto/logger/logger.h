#ifndef EXACTO_LOGGER_H_
#define EXACTO_LOGGER_H_

#include <stdint.h>

extern int exlg_updateLogger();
extern uint8_t exlg_uploadLogger(uint8_t * data, uint16_t datalen);

#endif //EXACTO_LOGGER_H_