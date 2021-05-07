/**
 * @file MessageHandlers.h
 * @author GubskiyAS (GubskiyAS@mail.ru)
 * @brief Заголовный файл обработчиков сообщений (Файл должен заменяться на свой)
 * @version 0.1
 * @date 2020-12-08
 * 
 * @copyright Copyright (c) 2020
 * 
 */
#ifndef MESSAGE_HANDLERS_H
#define MESSAGE_HANDLERS_H



#include <stdint.h>
#include "BuffersManager.h"
#include "ExactoProtocol_defs.h"
#include "exacto_MessageSettingsLsm303.h"
#include "exacto_MessageDataLsm303.h"
#include "exacto_MessageSettingsIsm330.h"
#include "exacto_MessageDataIsm330.h"
#include "exacto_MessageSettingsBmp280.h"
#include "exacto_MessageDataBmp280.h"

void handlerMessageSettingsLsm303(StructSettingsLsm303_t * struct_settings_lsm303, FieldTypeInExactolinkPacket_t TypeAccess);
void handlerMessageDataLsm303(StructDataLsm303_t * struct_data_lsm303, FieldTypeInExactolinkPacket_t TypeAccess);
void handlerMessageSettingsIsm330(StructSettingsIsm330_t * struct_settings_Ism330, FieldTypeInExactolinkPacket_t TypeAccess);
void handlerMessageDataIsm330(StructDataIsm330_t * struct_data_Ism330, FieldTypeInExactolinkPacket_t TypeAccess);
void handlerMessageSettingsBmp280(StructSettingsBmp280_t * struct_settings_Bmp280, FieldTypeInExactolinkPacket_t TypeAccess);
void handlerMessageDataBmp280(StructDataBmp280_t * struct_data_Bmp280, FieldTypeInExactolinkPacket_t TypeAccess);

uint8_t  getPriority(void); 		
uint32_t getDataSource(void); 	
uint64_t getTimeStamp(void);
uint32_t getRateMeas(void);
uint32_t getRange(void);

#endif
