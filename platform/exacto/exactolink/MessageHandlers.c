/**
 * @file MessageHandlers.c
 * @author GubskiyAS (GubskiyAS@mail.ru)
 * @brief пример реализации обработчиков конкретных сообщений. Подразумевается что для каждого датчика
 * будет создана своя пара файлов, но для упрощения примера все сведено в одну.
 * @version 0.1
 * @date 2020-12-08
 * 
 * @copyright Copyright (c) 2020
 * 
 */

#include <stdint.h>
#include <string.h>
#include "ExactoProtocol_defs.h"
#include "exacto_MessageDataLsm303.h"
#include "exacto_MessageSettingsLsm303.h"
#include "exacto_MessageDataIsm330.h"
#include "exacto_MessageSettingsIsm330.h"
#include "exacto_MessageDataBmp280.h"
#include "exacto_MessageSettingsBmp280.h"
#include "DebugformattedOut.h"
#include "Lsm303_registers.h"
#include "Ism330_registers.h"
#include "Bmp280_registers.h"
#include "exactolinkTypes.h"

/********************************	Variables 	********************************/


/********************************	Functions 	********************************/
void generateRandomData(uint8_t * ptrForRandomData, uint32_t size)
{

}

/*
 *
 *
 * 												LSM303
 *
 *
 *
 */

/**
 * @brief Функция записи настрочных параметров в датчик Lsm303. Функция будет использоваться в измерительном модуле.
 *
 * @param struct_settings_lsm303 - указатель на данные типа struct_SettingsLsm303_t (единичная установка регистра)
 */
void CommanderLsm303(StructSettingsLsm303_t * struct_settings_lsm303, FieldTypeInExactolinkPacket_t TypeAccess)
{
	//отправка датчику по SPI команды находящейся в struct_settings_lsm303
	if (TypeAccess == READ)
	{
		// StructSettingsLsm303_t local_struct_settings_lsm303;
		// local_struct_settings_lsm303.trgReg = ((struct_settings_lsm303->trgReg>>1 ) & ~LSM303_MODE_BIT_MASK) | LSM303_READ_MODE;//добавляем бит режима( запись = 0   или чтение = 1)
		// local_struct_settings_lsm303.valToReg = struct_settings_lsm303->valToReg;
		uint8_t recvData;
		/*
		 * реализация заменяется на
		 * SPI_HandleTypeDef * hspi1;
		 * HAL_SPI_Transmit_DMA(hspi1, (uint8_t *) &local_struct_settings_lsm303, sizeof(local_struct_settings_lsm303));
		 * HAL_SPI_Receive_DMA(hspi1, &recvData, sizeof(struct_settings_lsm303->valToReg));
		*/
		generateRandomData(&recvData, sizeof(struct_settings_lsm303->valToReg));
		//struct_settings_lsm303->trgReg - остался прежним
		struct_settings_lsm303->valToReg = recvData;// записали в структуру якобы принятые от датчика данные
		packMessageSettingsLsm303(struct_settings_lsm303, uTrue);
		formattedOut("CommanderLsm303(read register) ", (uint8_t *) struct_settings_lsm303, sizeof(StructSettingsLsm303_t));

	}
	else if (TypeAccess == WRITE)
	{
		formattedOut("CommanderLsm303(write register) ", (uint8_t *) struct_settings_lsm303, sizeof(StructSettingsLsm303_t));
		// StructSettingsLsm303_t local_struct_settings_lsm303;
		// local_struct_settings_lsm303.trgReg = ((struct_settings_lsm303->trgReg>>1 ) & ~LSM303_MODE_BIT_MASK) | LSM303_WRITE_MODE;//добавляем бит режима( запись = 0   или чтение = 1)
		// local_struct_settings_lsm303.valToReg = struct_settings_lsm303->valToReg;
		struct_settings_lsm303->trgReg = ((struct_settings_lsm303->trgReg>>1 ) & ~LSM303_MODE_BIT_MASK);//добавляем бит режима( запись = 0   или чтение = 1)
		/*
		 * реализация заменяется на
		 * SPI_HandleTypeDef * hspi1;
		 * HAL_SPI_Transmit_DMA(hspi1, (uint8_t *) &local_struct_settings_lsm303, sizeof(local_struct_settings_lsm303));
		*/
	}
	else exlnk_errorHook();
}



/**
 * @brief Обработчик команды чтения регистров датчика. Функция будет использоваться в измерительном модуле.
 *
 * @param struct_data_lsm303 - указатель на хранилице куда будут складываться прочитанные данные
 */
void ReporterLsm303(StructDataLsm303_t * ptr_struct_data_lsm303)
{
	formattedOut("ReporterLsm303(read data) ", (uint8_t *) ptr_struct_data_lsm303, sizeof(StructDataLsm303_t));
	//отправка в SPI struct_settings_lsm303 и прием соответствующего байта данных
	//полученный байт данных упаковывается в структуру answer
	//эмуляция отправки команды чтения выходных регистров

/*	реализация заменяется на
 * 	uint16_t register_address = LSM303_OUT_X_L_A_ADDRESS;
 * 	HAL_SPI_TransmitReceive_DMA(hspi, register_address, &struct_DataLsm303.accx, 1);
 * 	register_address = OUT_X_H_A_REGISTER_ADDRESS;
 *  HAL_SPI_TransmitReceive_DMA(hspi, register_address, (&struct_DataLsm303.accx)+1, 1);
 *  .... и т.д.
 */
	generateRandomData((uint8_t *) &ptr_struct_data_lsm303->accx, sizeof(ptr_struct_data_lsm303->accx));
	generateRandomData((uint8_t *) &ptr_struct_data_lsm303->accy, sizeof(ptr_struct_data_lsm303->accy));
	generateRandomData((uint8_t *) &ptr_struct_data_lsm303->accz, sizeof(ptr_struct_data_lsm303->accz));

	packMessageDataLsm303(ptr_struct_data_lsm303, uTrue);

}


/**
 * @brief Обработчик сообщения с настройками (запись либо чтение либо ответ на чтение). на разных устрйоствах будут приходить разного типа TypeAccess сообщения
 * 
 * @param struct_settings_lsm303 - указатель на данные типа struct_SettingsLsm303_t (единичная установка регистра)
 * @param TypeAccess - тип данных (SIGNAL, WRITE, READ, ANSWER)
 */
void handlerMessageSettingsLsm303(StructSettingsLsm303_t * ptr_struct_settings_lsm303, FieldTypeInExactolinkPacket_t TypeAccess)
{
	formattedOut("handlerMessageSettingsLsm303() enter <-", (uint8_t *) ptr_struct_settings_lsm303, sizeof(StructSettingsLsm303_t));
		/*пример реализации*/	
	switch(TypeAccess)
	{
		case WRITE:
		case READ:
			CommanderLsm303(ptr_struct_settings_lsm303, TypeAccess); 	//отправка настроечных комманд. такие сообщения будут приниматься на измерительном модуле
			break;
		case ANSWER:
			formattedOut("Recieved ANSWER Message with Settings Lsm303 ", (uint8_t *) ptr_struct_settings_lsm303, sizeof(StructSettingsLsm303_t));
			//такие данные будут приходить на модуле "система" и их надо будет обработать
			break;
		case SIGNAL:
			//такие данные будут приходить на модуле и их надо будет обработать
			break;
		default:
			exlnk_errorHook();//приход такого типа сообщения в данном обработчике не предполагается
			break;
	}

//	struct_SettingsLsm303_t struct_settings_lsm303;	
//	packMessageSettingsLsm303(&struct_settings_lsm303);  //можно вызвать непосредственно упаковщик для записи сформированного сообщения
	formattedOut("handlerMessageSettingsLsm303() exit ->", (uint8_t *) ptr_struct_settings_lsm303, 0);
}

/**
 * @brief Обработчик сообщения для данных. (команда на чтение либо ответ с данными)
 * 
 * @param struct_data_lsm303 - указатель на данные типа struct_DataLsm303_t (выборка данных)
 * @param TypeAccess - тип данных (SIGNAL, WRITE, READ, ANSWER)
 */
void handlerMessageDataLsm303(StructDataLsm303_t * ptr_struct_data_lsm303, FieldTypeInExactolinkPacket_t TypeAccess)
{
	formattedOut("handlerMessageDataLsm303() enter <-", (uint8_t *) ptr_struct_data_lsm303, sizeof(StructDataLsm303_t));
			/*пример реализации*/	
	switch(TypeAccess)
	{
		case READ:
			ReporterLsm303(ptr_struct_data_lsm303); 		//команда прочесть данные (такое сообщение приходит на измерительном модуле)
			break;
		case ANSWER:
			formattedOut("Recieved ANSWER Message with Data Lsm303 ", (uint8_t *) ptr_struct_data_lsm303, sizeof(StructDataLsm303_t));
			//такие данные будут приходить на модуле "система" и их надо будет обработать
			break;

		case SIGNAL:
		case WRITE:
		default:
			exlnk_errorHook();//приход такого типа сообщения в данном обработчике не предполагается
			break;
	}

//	struct_DataLsm303_t struct_data_lsm303;	
//	packMessageDataLsm303(&struct_data_lsm303);  //можно вызвать непосредственно упаковщик для записи сформированного сообщения 
	formattedOut("handlerMessageDataLsm303() exit ->", (uint8_t *) ptr_struct_data_lsm303, 0);
}





/*
 *
 *
 * 												ISM330
 *
 *
 *
 */

/**
 * @brief Функция записи настрочных параметров в датчик Ism330. Функция будет использоваться в измерительном модуле.
 *
 * @param struct_settings_ism330 - указатель на данные типа struct_SettingsIsm330_t (единичная установка регистра)
 * @param TypeAccess - тип данных (SIGNAL, WRITE, READ, ANSWER)
 */
void CommanderIsm330(StructSettingsIsm330_t * struct_settings_ism330, FieldTypeInExactolinkPacket_t TypeAccess)
{
	//отправка датчику по SPI команды находящейся в struct_settings_ism330
	if (TypeAccess == READ)
	{
		// StructSettingsIsm330_t local_struct_settings_ism330;
		// local_struct_settings_ism330.trgReg = ((struct_settings_ism330->trgReg<<1 ) & ~ISM330_MODE_BIT_MASK) | ISM330_READ_MODE;//добавляем бит режима( запись = 0   или чтение = 1)
		// local_struct_settings_ism330.valToReg = struct_settings_ism330->valToReg;
		uint8_t recvData;
		/*
		 * реализация заменяется на
		 * SPI_HandleTypeDef * hspi1;
		 * HAL_SPI_Transmit_DMA(hspi1, (uint8_t *) &local_struct_settings_ism330, sizeof(local_struct_settings_ism330));
		 * HAL_SPI_Receive_DMA(hspi1, &recvData, sizeof(struct_settings_ism330->valToReg));
		*/
		generateRandomData(&recvData, sizeof(struct_settings_ism330->valToReg));
		//struct_settings_ism330->trgReg - остался прежним
		struct_settings_ism330->valToReg = recvData;// записали в структуру якобы принятые от датчика данные
		packMessageSettingsIsm330(struct_settings_ism330, uTrue);
		formattedOut("CommanderIsm330(read register) ", (uint8_t *) struct_settings_ism330, sizeof(StructSettingsIsm330_t));
	}
	else if (TypeAccess == WRITE)
	{
		formattedOut("CommanderIsm330(write register) ", (uint8_t *) struct_settings_ism330, sizeof(StructSettingsIsm330_t));
		// StructSettingsIsm330_t local_struct_settings_ism330;
		// local_struct_settings_ism330.trgReg = ((struct_settings_ism330->trgReg<<1 ) & ~ISM330_MODE_BIT_MASK) | ISM330_WRITE_MODE;//добавляем бит режима( запись = 0   или чтение = 1)
		// local_struct_settings_ism330.valToReg = struct_settings_ism330->valToReg;
		struct_settings_ism330->trgReg = ((struct_settings_ism330->trgReg>>1 ) & ~ISM330_MODE_BIT_MASK);//добавляем бит режима( запись = 0   или чтение = 1)
		/*
		 * реализация заменяется на
		 * SPI_HandleTypeDef * hspi1;
		 * HAL_SPI_Transmit_DMA(hspi1, (uint8_t *) &local_struct_settings_ism330, sizeof(local_struct_settings_ism330));
		*/
	}
	else exlnk_errorHook();
}



/**
 * @brief Обработчик команды чтения регистров датчика. Функция будет использоваться в измерительном модуле.
 *
 * @param struct_data_ism330 - указатель на хранилице куда будут складываться прочитанные данные
 */
void ReporterIsm330(StructDataIsm330_t * ptr_struct_data_ism330)
{
	formattedOut("ReporterIsm330(read data ISM330) ", (uint8_t *) ptr_struct_data_ism330, sizeof(StructDataIsm330_t));
	//отправка в SPI struct_settings_ism330 и прием соответствующего байта данных
	//полученный байт данных упаковывается в структуру answer
	//эмуляция отправки команды чтения выходных регистров

/*	реализация заменяется на
 * 	uint16_t register_address = ISM330_OUT_X_L_A_ADDRESS;
 * 	HAL_SPI_TransmitReceive_DMA(hspi, register_address, &struct_DataIsm330.accx, 1);
 * 	register_address = OUT_X_H_A_REGISTER_ADDRESS;
 *  HAL_SPI_TransmitReceive_DMA(hspi, register_address, (&struct_DataIsm330.accx)+1, 1);
 *  .... и т.д.
 */
	generateRandomData((uint8_t *) &ptr_struct_data_ism330->accx, sizeof(ptr_struct_data_ism330->accx));
	generateRandomData((uint8_t *) &ptr_struct_data_ism330->accy, sizeof(ptr_struct_data_ism330->accy));
	generateRandomData((uint8_t *) &ptr_struct_data_ism330->accz, sizeof(ptr_struct_data_ism330->accz));

	packMessageDataIsm330(ptr_struct_data_ism330, uTrue);
}


/**
 * @brief Обработчик сообщения с настройками (запись либо чтение либо ответ на чтение). на разных устрйоствах будут приходить разного типа TypeAccess сообщения
 *
 * @param struct_settings_ism330 - указатель на данные типа struct_SettingsIsm330_t (единичная установка регистра)
 * @param TypeAccess - тип данных (SIGNAL, WRITE, READ, ANSWER)
 */
void handlerMessageSettingsIsm330(StructSettingsIsm330_t * ptr_struct_settings_ism330, FieldTypeInExactolinkPacket_t TypeAccess)
{
	formattedOut("handlerMessageSettingsIsm330() enter <-", (uint8_t *) ptr_struct_settings_ism330, sizeof(StructSettingsIsm330_t));
		/*пример реализации*/
	switch(TypeAccess)
	{
		case WRITE:
		case READ:
			CommanderIsm330(ptr_struct_settings_ism330, TypeAccess); 	//отправка настроечных комманд. такие сообщения будут приниматься на измерительном модуле
			break;
		case ANSWER:
			//такие данные будут приходить на модуле "система" и их надо будет обработать
			formattedOut("Recieved ANSWER Message with Settings Ism330 ", (uint8_t *) ptr_struct_settings_ism330, sizeof(StructSettingsIsm330_t));
			break;
		case SIGNAL:
			//такие данные будут приходить на модуле и их надо будет обработать
			break;
		default:
			exlnk_errorHook();//приход такого типа сообщения в данном обработчике не предполагается
			break;
	}

//	struct_SettingsIsm330_t struct_settings_ism330;
//	packMessageSettingsIsm330(&struct_settings_ism330);  //можно вызвать непосредственно упаковщик для записи сформированного сообщения
	formattedOut("handlerMessageSettingsIsm330() exit ->", (uint8_t *) ptr_struct_settings_ism330, 0);
}

/**
 * @brief Обработчик сообщения для данных. (команда на чтение либо ответ с данными)
 *
 * @param struct_data_ism330 - указатель на данные типа struct_DataIsm330_t (выборка данных)
 * @param TypeAccess - тип данных (SIGNAL, WRITE, READ, ANSWER)
 */
void handlerMessageDataIsm330(StructDataIsm330_t * ptr_struct_data_ism330, FieldTypeInExactolinkPacket_t TypeAccess)
{
	formattedOut("handlerMessageDataIsm330() enter <-", (uint8_t *) ptr_struct_data_ism330, sizeof(StructDataIsm330_t));
			/*пример реализации*/
	switch(TypeAccess)
	{
		case READ:
			ReporterIsm330(ptr_struct_data_ism330); 		//команда прочесть данные (такое сообщение приходит на измерительном модуле)
			break;
		case ANSWER:
			//такие данные будут приходить на модуле "система" и их надо будет обработать
			formattedOut("Recieved ANSWER Message with Data Ism330 ", (uint8_t *) ptr_struct_data_ism330, sizeof(StructDataIsm330_t));
			break;

		case SIGNAL:
		case WRITE:
		default:
			exlnk_errorHook();//приход такого типа сообщения в данном обработчике не предполагается
			break;
	}

//	struct_DataIsm330_t struct_data_ism330;
//	packMessageDataIsm330(&struct_data_ism330);  //можно вызвать непосредственно упаковщик для записи сформированного сообщения
	formattedOut("handlerMessageDataIsm330() exit ->", (uint8_t *) ptr_struct_data_ism330, 0);
}





/*
 *
 *
 * 												BMP280
 *
 *
 *
 */

/**
 * @brief Функция записи настрочных параметров в датчик Bmp280. Функция будет использоваться в измерительном модуле.
 *
 * @param struct_settings_bmp280 - указатель на данные типа struct_SettingsBmp280_t (единичная установка регистра)
 */
void CommanderBmp280(StructSettingsBmp280_t * struct_settings_bmp280, FieldTypeInExactolinkPacket_t TypeAccess)
{
	//отправка датчику по SPI команды находящейся в struct_settings_bmp280
	if (TypeAccess == READ)
	{
		// StructSettingsBmp280_t local_struct_settings_bmp280;
		// local_struct_settings_bmp280.trgReg = ((struct_settings_bmp280->trgReg<<1 ) & ~BMP280_MODE_BIT_MASK) | BMP280_READ_MODE;//добавляем бит режима( запись = 0   или чтение = 1)
		// local_struct_settings_bmp280.valToReg = struct_settings_bmp280->valToReg;
		uint8_t recvData;
		/*
		 * реализация заменяется на
		 * SPI_HandleTypeDef * hspi1;
		 * HAL_SPI_Transmit_DMA(hspi1, (uint8_t *) &local_struct_settings_bmp280, sizeof(local_struct_settings_bmp280));
		 * HAL_SPI_Receive_DMA(hspi1, &recvData, sizeof(struct_settings_bmp280->valToReg));
		*/
		formattedOut("CommanderBmp280(read register) enter <-", (uint8_t *) struct_settings_bmp280, sizeof(StructSettingsBmp280_t));
		generateRandomData(&recvData, sizeof(struct_settings_bmp280->valToReg));
		//struct_settings_bmp280->trgReg - остался прежним
		struct_settings_bmp280->valToReg = recvData;// записали в структуру якобы принятые от датчика данные
		packMessageSettingsBmp280(struct_settings_bmp280, uTrue);
		formattedOut("CommanderBmp280(read register) exit ->", (uint8_t *) struct_settings_bmp280, sizeof(StructSettingsBmp280_t));
	}
	else if (TypeAccess == WRITE)
	{
		formattedOut("CommanderBmp280(write register) ", (uint8_t *) struct_settings_bmp280, sizeof(StructSettingsBmp280_t));
		// StructSettingsBmp280_t local_struct_settings_bmp280;
		// local_struct_settings_bmp280.trgReg = ((struct_settings_bmp280->trgReg<<1 ) & ~BMP280_MODE_BIT_MASK) | BMP280_WRITE_MODE;//добавляем бит режима( запись = 0   или чтение = 1)
		// local_struct_settings_bmp280.valToReg = struct_settings_bmp280->valToReg;
		struct_settings_bmp280->trgReg = ((struct_settings_bmp280->trgReg>>1 ) & ~BMP280_MODE_BIT_MASK);//добавляем бит режима( запись = 0   или чтение = 1)
		/*
		 * реализация заменяется на
		 * SPI_HandleTypeDef * hspi1;
		 * HAL_SPI_Transmit_DMA(hspi1, (uint8_t *) &local_struct_settings_bmp280, sizeof(local_struct_settings_bmp280));
		*/
	}
	else exlnk_errorHook();
}



/**
 * @brief Обработчик команды чтения регистров датчика. Функция будет использоваться в измерительном модуле.
 *
 * @param struct_data_bmp280 - указатель на хранилице куда будут складываться прочитанные данные
 */
void ReporterBmp280(StructDataBmp280_t * ptr_struct_data_bmp280)
{
	formattedOut("ReporterBmp280(read data BMP280) ", (uint8_t *) ptr_struct_data_bmp280, sizeof(StructDataBmp280_t));
	//отправка в SPI struct_settings_bmp280 и прием соответствующего байта данных
	//полученный байт данных упаковывается в структуру answer
	//эмуляция отправки команды чтения выходных регистров

/*	реализация заменяется на
 * 	uint16_t register_address = BMP280_OUT_X_L_A_ADDRESS;
 * 	HAL_SPI_TransmitReceive_DMA(hspi, register_address, &struct_DataBmp280.accx, 1);
 * 	register_address = OUT_X_H_A_REGISTER_ADDRESS;
 *  HAL_SPI_TransmitReceive_DMA(hspi, register_address, (&struct_DataBmp280.accx)+1, 1);
 *  .... и т.д.
 */
	generateRandomData((uint8_t *) &ptr_struct_data_bmp280->accx, sizeof(ptr_struct_data_bmp280->accx));
	generateRandomData((uint8_t *)&ptr_struct_data_bmp280->accy, sizeof(ptr_struct_data_bmp280->accy));
	generateRandomData((uint8_t *)&ptr_struct_data_bmp280->accz, sizeof(ptr_struct_data_bmp280->accz));

	packMessageDataBmp280(ptr_struct_data_bmp280, uTrue);
}


/**
 * @brief Обработчик сообщения с настройками (запись либо чтение либо ответ на чтение). на разных устрйоствах будут приходить разного типа TypeAccess сообщения
 *
 * @param struct_settings_bmp280 - указатель на данные типа struct_SettingsBmp280_t (единичная установка регистра)
 * @param TypeAccess - тип данных (SIGNAL, WRITE, READ, ANSWER)
 */
void handlerMessageSettingsBmp280(StructSettingsBmp280_t * ptr_struct_settings_bmp280, FieldTypeInExactolinkPacket_t TypeAccess)
{
	formattedOut("handlerMessageSettingsBmp280() enter <-", (uint8_t *) ptr_struct_settings_bmp280, sizeof(StructSettingsBmp280_t));
		/*пример реализации*/
	switch(TypeAccess)
	{
		case WRITE:
		case READ:
			CommanderBmp280(ptr_struct_settings_bmp280, TypeAccess); 	//отправка настроечных комманд. такие сообщения будут приниматься на измерительном модуле
			break;
		case ANSWER:
			//такие данные будут приходить на модуле "система" и их надо будет обработать
			formattedOut("Recieved ANSWER Message with Settings Bmp280 ", (uint8_t *) ptr_struct_settings_bmp280, sizeof(StructSettingsBmp280_t));
			break;
		case SIGNAL:
			//такие данные будут приходить на модуле и их надо будет обработать
			break;
		default:
			exlnk_errorHook();//приход такого типа сообщения в данном обработчике не предполагается
			break;
	}

//	struct_SettingsBmp280_t struct_settings_bmp280;
//	packMessageSettingsBmp280(&struct_settings_bmp280);  //можно вызвать непосредственно упаковщик для записи сформированного сообщения
	formattedOut("handlerMessageSettingsBmp280() exit ->", (uint8_t *) ptr_struct_settings_bmp280, 0);
}

/**
 * @brief Обработчик сообщения для данных. (команда на чтение либо ответ с данными)
 *
 * @param struct_data_bmp280 - указатель на данные типа struct_DataBmp280_t (выборка данных)
 * @param TypeAccess - тип данных (SIGNAL, WRITE, READ, ANSWER)
 */
void handlerMessageDataBmp280(StructDataBmp280_t * ptr_struct_data_bmp280, FieldTypeInExactolinkPacket_t TypeAccess)
{
	formattedOut("handlerMessageDataBmp280() enter <-", (uint8_t *) ptr_struct_data_bmp280, sizeof(StructDataBmp280_t));
			/*пример реализации*/
	switch(TypeAccess)
	{
		case READ:
			ReporterBmp280(ptr_struct_data_bmp280); 		//команда прочесть данные (такое сообщение приходит на измерительном модуле)
			break;
		case ANSWER:
			//такие данные будут приходить на модуле "система" и их надо будет обработать
			formattedOut("Recieved ANSWER Message with Data Bmp280 ", (uint8_t *) ptr_struct_data_bmp280, sizeof(StructDataBmp280_t));
			break;

		case SIGNAL:
		case WRITE:
		default:
			exlnk_errorHook();//приход такого типа сообщения в данном обработчике не предполагается
			break;
	}

//	struct_DataBmp280_t struct_data_bmp280;
//	packMessageDataBmp280(&ptr_struct_data_bmp280);  //можно вызвать непосредственно упаковщик для записи сформированного сообщения
	formattedOut("handlerMessageDataBmp280() exit ->", (uint8_t *) ptr_struct_data_bmp280, 0);
}











/**
 * @brief Get the Priority message
 * 
 * @return uint8_t 
 */
uint8_t  getPriority()
{
	return 11;
}
/**
 * @brief Get the Data Source message
 * 
 * @return uint32_t 
 */
uint32_t getDataSource()
{
	return 0x12345678ul;
}
/**
 * @brief Get the Time Stamp message
 * 
 * @return uint64_t 
 */
uint64_t getTimeStamp(void)
{
	return 0;
}
/**
 * @brief Get the Rate Meas 
 * 
 * @return uint32_t 
 */
uint32_t getRateMeas(void)
{
	return 1;
}
/**
 * @brief Get the Range
 * 
 * @return uint32_t 
 */
uint32_t getRange(void)
{
	return 0;
}
