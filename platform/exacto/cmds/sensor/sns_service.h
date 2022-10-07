#ifndef SNS_SERVICE_H_
#define SNS_SERVICE_H_
#include <stdint.h>
#include "commander/exacto_sns_ctrl.h"

extern ex_sns_lth_container_t SNSSRV_SendAndUpload_Container;
extern uint8_t ex_switchStage_SnsService(exactolink_package_result_t type);

#endif
