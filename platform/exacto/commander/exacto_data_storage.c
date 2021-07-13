#include "exacto_data_storage.h"
#include "hardtools/basic.h"
#include <embox/unit.h>
// #include "blink/blinker.h"
#include <kernel/printk.h>

#define EXACTO_DATA_STORAGE_TEST

uint32_t ExDtStr_TransmitSPI_Counter = 0;
uint32_t ExDtStr_TransmitSPI_TxCounter = 0;
uint32_t ExDtStr_TransmitSPI_RxCounter = 0;
uint32_t EDS_SPI_pullcount = 0;

exactolink_package_result_t EDS_CurrentExactolinkType;

//temporary
exactolink_package_info_t ExDtStr_TrasmitSPI_Info = {
    .is_data_available = 0,
    .packagetype = EXACTOLINK_NO_DATA,
};
exactolink_package_info_t ExDtStr_TrasmitSPI_Info_tmp = {
    .is_data_available = 0,
    .packagetype = EXACTOLINK_NO_DATA,
};
uint32_t ExDtStr_TrasmitSPI_RefCounter = 0;
uint32_t ExDtStr_TrasmitSPI_RefCounterPrev = 0;

uint32_t ExDtStr_TrasmitSPI_LostCnt = 0;
uint32_t ExDtStr_TrasmitSPI_DbleCnt = 0;
uint32_t ExDtStr_TrasmitSPI_OverFlw = 0;

uint16_t ExDtStr_OutputSPI_OverFlw = 0;

static uint32_t EDS_DataStorage_UdtCnt = 0;
/**
 * @brief store info and data about external input
 * 
 */
exactodatastorage ExDtStorage = {
    .isEmpty = 1,
};
thread_control_t ExOutputStorage[THREAD_OUTPUT_TYPES_SZ]; 
ExactoBufferExtended ExDtStr_SD_buffer;

 ex_subs_service_t ExDataStorageServices[SERVICES_COUNT];
 ex_service_info_t ExDataStorageServicesInfo = {
     .current_count = 0,
     .max_count = SERVICES_COUNT,
 };

thread_control_t SetupParamsThread;

thread_control_t TickReactionThread = {
    .datalen = 0,
    .datamaxcount = 10,
    .result = THR_CTRL_WAIT,
};

uint8_t ex_setExactolinkType( exactolink_package_result_t new_type)
{
    EDS_CurrentExactolinkType = new_type;
    return 0;
}
uint8_t ex_getExactolinkType( exactolink_package_result_t * type)
{
    *type = EDS_CurrentExactolinkType;
    return 0;
}
static int runTickReactionThread(struct lthread * self)
{
    TickReactionThread.datalen++;
    if (TickReactionThread.datalen > TickReactionThread.datamaxcount)
    {
        if (TickReactionThread.result == THR_CTRL_WAIT)
        {
            // setSysLedOn();
            TickReactionThread.result = THR_CTRL_OK;
        }
        else
        {
            // setSysLedOff();
            TickReactionThread.result = THR_CTRL_WAIT;
        }
        TickReactionThread.datalen = 0;
    }
    return 0;
}

void startTickReactionThread( )
{
    lthread_launch(&TickReactionThread.thread);
}

static int setupParamsThreadRun(struct lthread * self)
{
    thread_control_t * _trg_thread = (thread_control_t *) self;
    ExOutputStorage[THR_SPI_RX].datalen = _trg_thread->datalen;
    return 0;
}

struct lthread ResetThread;
ex_io_thread_t ExSpi = {
    .isready = 0,
    .isenabled = 0
};


static int resetThreadRun(struct lthread * self)
{
    ExDtStorage.isEmpty = 1;
    for (uint8_t i = 0 ; i < THREAD_OUTPUT_TYPES_SZ; i++)
    {
        ExOutputStorage[i].result = THR_CTRL_NO_RESULT;
        ExOutputStorage[i].isready = 0;
        ExOutputStorage[i].datamaxcount = THREAD_CONTROL_BUFFER_SZ;
        setini_exbu8(&ExOutputStorage[i].datastorage);
    }
return 0;
}


/**
 * @brief save check data 
 * 
 * @param self thread with implemented data storage
 * @return int 
 */
static int functionForExDtStorageHandler(struct lthread *self)
{
    thread_control_t *_trg_lthread;
    _trg_lthread = (thread_control_t*)self;
    goto *lthread_resume(self, &&start);
start:
     /* инициализация */

mutex_retry:
    // do       something
    // if (mutex_trylock_lthread(self, &_trg_lthread->mx ) == -EAGAIN)
    // {
    //     return lthread_yield(&&start, &&mutex_retry);
    // }
    // //===============================================================
    if (mutex_trylock_lthread(self, &ExDtStorage.dtmutex ) == -EAGAIN)
    {
        return lthread_yield(&&start, &&mutex_retry);
    }
    _trg_lthread->result = THR_CTRL_NO_RESULT;

    if (!ExDtStorage.isEmpty) 
    {
        _trg_lthread->result = THR_CTRL_OK;
    }

    mutex_unlock_lthread(self, &ExDtStorage.dtmutex);
    // //===============================================================
    // mutex_unlock_lthread(self, &_trg_lthread->mx);

    // after    something
    return 0;
}



EMBOX_UNIT_INIT(initExactoDataStorage);
/**
 * @brief инициирует хранилище данных, в данный момент, только мьютекс
 * 
 * @return int всегда ноль
 */
static int initExactoDataStorage(void)
{
    mutex_init_schedee(&ExDtStorage.dtmutex);
    lthread_init(&ResetThread, resetThreadRun);
    lthread_init(&SetupParamsThread.thread, setupParamsThreadRun);
    lthread_init(&TickReactionThread.thread, runTickReactionThread);
    ExOutputStorage[0].type = THR_SPI_RX;
    ExOutputStorage[1].type = THR_SPI_TX;
    ExOutputStorage[2].type = THR_STR_CALC_IN;
    ExOutputStorage[3].type = THR_STR_CALC_OUT;
    setini_exbextu8(&ExDtStr_SD_buffer);
    for (uint8_t i = 0 ; i < THREAD_OUTPUT_TYPES_SZ; i++)
    {
        ExOutputStorage[i].result = THR_CTRL_NO_RESULT;
        ExOutputStorage[i].isready = 0;
        ExOutputStorage[i].datamaxcount = THREAD_CONTROL_BUFFER_SZ;
        setini_exbu8(&ExOutputStorage[i].datastorage);
    }
    for (uint8_t i = 0 ; i < ExDataStorageServicesInfo.max_count; i++)
    {
        ExDataStorageServices[0].isenabled = 0;
        ExDataStorageServices[0].type = THR_NONE;
    }
    return 0;
}
/**
 * @brief инициирует легкий поток для проверки наличия данных
 * 
 * @param base легкий поток
 * @return uint8_t 
 */
uint8_t checkExactoDataStorage( thread_control_t * base)
{
    lthread_launch(&base->thread);
    return 0;
}
/**
 * @brief подготавливает легкий поток для работы
 * 
 * @param base легкий поток
 * @return uint8_t 
 */
uint8_t initThreadExactoDataStorage( thread_control_t * base )
{
    mutex_init_schedee(&base->mx);
    lthread_init(&base->thread, functionForExDtStorageHandler);
    base->result = THR_CTRL_WAIT;
    return 0;
}
uint8_t transmitExactoDataStorage()
{
    // if (ExOutputStorage[THR_SPI_TX].isready)
    // {
    // ex_updateCounter_ExDtStr(THR_SPI_TX);
    lthread_launch(&ExOutputStorage[THR_SPI_TX].thread);
    // }
    return 0;
}
uint8_t receiveExactoDataStorage()
{
    // if (ExOutputStorage[THR_SPI_RX].isready)
    // {
    // ex_updateCounter_ExDtStr(THR_SPI_RX);
    lthread_launch(&ExOutputStorage[THR_SPI_RX].thread);
    // }
    return 0;
}
uint8_t clearExactoDataStorage()
{
    setemp_exbu8(&ExOutputStorage[THR_SPI_TX].datastorage);
    return 0;
}
void setHeaderExactoDataStorage(const uint8_t type, const uint16_t address, const uint16_t length)
{
    // HEADER
    //[00] 0x17 
    //[01] 0x17
    //----------------------
    //Datalen
    //[02] 0x00
    //[03] 0x00
    //----------------------
    //TYPE
    //[04] 
    //----------------------
    //pointer on data start
    //[05] 0x09 
    //----------------------
    //ADDRESS
    //[06]
    //[07]
    //----------------------
    //DATA PACKAGE
    //[08] 0x00

    const uint8_t pck_id = EXACTOLINK_PCK_ID;
    const uint8_t addrH = (uint8_t) (address << 8);
    const uint8_t addrL = (uint8_t) (address);
    const uint8_t lenH = (uint8_t) (length << 8);
    const uint8_t lenL = (uint8_t) (length);
    const uint8_t data_start_point = EXACTOLINK_START_DATA_POINT_VAL;
    pshfrc_exbu8(&ExOutputStorage[THR_SPI_TX].datastorage, pck_id);             //[0]
    pshfrc_exbu8(&ExOutputStorage[THR_SPI_TX].datastorage, lenH);               //[1]
    pshfrc_exbu8(&ExOutputStorage[THR_SPI_TX].datastorage, lenL);               //[2]
    pshfrc_exbu8(&ExOutputStorage[THR_SPI_TX].datastorage, type);               //[3]
    pshfrc_exbu8(&ExOutputStorage[THR_SPI_TX].datastorage, data_start_point);   //[4]
    pshfrc_exbu8(&ExOutputStorage[THR_SPI_TX].datastorage, addrH);              //[5]
    pshfrc_exbu8(&ExOutputStorage[THR_SPI_TX].datastorage, addrL);              //[6]
}
thread_control_result_t getStateExactoDataStorage()
{
    return ExOutputStorage[THR_SPI_TX].result;
}
void updateData2EDS(uint8_t value)
{
    // pshfrc_exbu8(&ExOutputStorage[THR_SPI_TX].datastorage, value);
    if (!pshsft_exbu8(&ExOutputStorage[THR_SPI_TX].datastorage, value))
    {
        ExDtStr_OutputSPI_OverFlw++;
    }
}
uint8_t setDataToExactoDataStorage(uint8_t * data, const uint16_t datacount, thread_control_result_t result)
{
    switch (result)
    {
    case THR_CTRL_INIT:
        /* code */
        // clearExactoDataStorage();
        //начало итерации записи данных
        break;
    case THR_CTRL_OK:
        //конец итерации записи
        break;
    case THR_CTRL_UNKNOWN_ERROR:
    case THR_CTRL_NO_RESULT:
        return 1;
    default:
        break;
    }
    for (uint16_t i = 0; i < datacount; i++)
    {
        // pshfrc_exbu8(&ExOutputStorage[THR_SPI_TX].datastorage, data[i]);
        updateData2EDS(data[i]);
        EDS_DataStorage_UdtCnt++;
    }
    ExOutputStorage[THR_SPI_TX].result = result;
    switch (EDS_CurrentExactolinkType)
    {
    case EXACTOLINK_LSM303AH_TYPE0:
        if (result == THR_CTRL_WAIT)
        {
            // pshfrc_exbu8(&ExOutputStorage[THR_SPI_TX].datastorage, 0x00);
            // pshfrc_exbu8(&ExOutputStorage[THR_SPI_TX].datastorage, 0x00);
            // updateData2EDS(0x00);
            // updateData2EDS(0x00);
        }
        break;
    default:
        break;
    }
    return 0;
}
uint8_t watchPackFromExactoDataStorage(uint8_t * receiver, const uint16_t receiver_length, uint8_t type)
{
    if (type == 0)
    {
        watchsvr_exbu8(&ExOutputStorage[THR_SPI_TX].datastorage, receiver, receiver_length);
    }
    else if (type == 1)
    {
        grbsvr_exbu8(&ExOutputStorage[THR_SPI_TX].datastorage, receiver, receiver_length);
    }
    return 0;
}
uint8_t getMailFromExactoDataStorage(uint8_t * receiver, const uint16_t receiver_length)
{
    if (!ExOutputStorage[THR_SPI_TX].isready)
        return 1;
    uint8_t type = EDS_CurrentExactolinkType;
    const uint8_t pck_id = EXACTOLINK_PCK_ID;
    uint16_t address = 1;
    const uint8_t addrH = (uint8_t) (address >> 8);
    const uint8_t addrL = (uint8_t) (address);
    uint8_t lenH, lenL, data_start_point;
    uint32_t crc;
    uint16_t data_body_length, length;
    switch (EDS_CurrentExactolinkType)
    {
    case EXACTOLINK_LSM303AH_TYPE0:
        //начало пакета
//         if (receiver_length <  EXACTOLINK_START_DATA_POINT_VAL + 4)
//             return 1;
//         data_body_length = getlen_exbu8(&ExOutputStorage[THR_SPI_TX].datastorage);
//         if (data_body_length > EXACTOLINK_APOLLON_SPI_PACK_SIZE)
//             data_body_length = EXACTOLINK_APOLLON_SPI_PACK_SIZE;
//         length =  EXACTOLINK_START_DATA_POINT_VAL + data_body_length + 4;
//         lenH = (uint8_t) (length >> 8);
//         lenL = (uint8_t) (length);
//         data_start_point = EXACTOLINK_START_DATA_POINT_VAL;
//         //header
//         receiver[0] = pck_id;
//         receiver[1] = lenL;
//         receiver[2] = lenH;
//         receiver[3] = type;
//         receiver[4] = data_start_point; //datatype
//         receiver[5] = 0;
//         receiver[6] = 0xff;  //priority
//         receiver[7] = addrL;
//         receiver[8] = addrH; //datasrc
//         //счетчик
//         receiver[9]  = (uint8_t) ExDtStr_TransmitSPI_TxCounter;    //counter
//         receiver[10] = (uint8_t)(ExDtStr_TransmitSPI_TxCounter >> 8);
//         receiver[11] = (uint8_t)(ExDtStr_TransmitSPI_TxCounter >> 16);
//         receiver[12] = (uint8_t)(ExDtStr_TransmitSPI_TxCounter >> 24);

//         for (uint16_t i = 0; i < ( data_body_length); 
//                                      i += EXACTOLINK_LSM303AH_TYPE0_ONE_INFOPACK_LENGTH)
//         {
//             for (uint16_t y = 0; y < EXACTOLINK_LSM303AH_TYPE0_ONE_INFOPACK_LENGTH; y++)
//             {
//                 uint8_t value;
//                 if(!grbfst_exbu8(&ExOutputStorage[THR_SPI_TX].datastorage, &value))
//                 {
//                     goto getMailFromExactoDataStorage_EXACTOLINK_LSM303AH_TYPE0_dataisempty_marker; //GOTO осторожно!!!
//                 }
//                 receiver[EXACTOLINK_START_DATA_POINT_VAL + i + y] = value;
//             }
            
//         }
// getMailFromExactoDataStorage_EXACTOLINK_LSM303AH_TYPE0_dataisempty_marker:
//         // grball_exbu8(&ExOutputStorage[THR_SPI_TX].datastorage, &receiver[EXACTOLINK_START_DATA_POINT_VAL]);
//         //очищение
//         // clearExactoDataStorage();
//         ex_getCRC(&receiver[0], (length - 4), &crc);
//         receiver[length - 4] = (uint8_t)(crc);
//         receiver[length - 3] = (uint8_t)(crc >> 8);
//         receiver[length - 2] = (uint8_t)(crc >> 16);
//         receiver[length - 1] = (uint8_t)(crc >> 24);        
//         break;
    case EXACTOLINK_SNS_XLXLGR:
        //начало пакета
        if (receiver_length <  EXACTOLINK_START_DATA_POINT_VAL + 4)
            return 1;
        data_body_length = getlen_exbu8(&ExOutputStorage[THR_SPI_TX].datastorage);
        if (data_body_length > EXACTOLINK_APOLLON_SPI_PACK_SIZE)
            data_body_length = EXACTOLINK_APOLLON_SPI_PACK_SIZE;
        data_start_point = EXACTOLINK_XLXLGR_START_DATA_POINT_VAL;
        length =  data_start_point + data_body_length + 4;
        lenH = (uint8_t) (length >> 8);
        lenL = (uint8_t) (length);
        //header
        receiver[0] = pck_id;
        receiver[1] = lenL;
        receiver[2] = lenH;
        //body header
        receiver[3] = type;
        receiver[4] = data_start_point; //datatype
        receiver[5] = 0;
        receiver[6] = 0xff;  //priority
        receiver[7] = addrL;
        receiver[8] = addrH; //datasrc
        //счетчик
        receiver[9]  = (uint8_t) ExDtStr_TransmitSPI_TxCounter;   
        receiver[10] = (uint8_t)(ExDtStr_TransmitSPI_TxCounter >> 8);
        receiver[11] = (uint8_t)(ExDtStr_TransmitSPI_TxCounter >> 16);
        receiver[12] = (uint8_t)(ExDtStr_TransmitSPI_TxCounter >> 24);
        
        receiver[13] = (uint8_t)(ExDtStr_OutputSPI_OverFlw );
        receiver[14] = (uint8_t)(ExDtStr_OutputSPI_OverFlw >> 8);
        receiver[15] = 0;
        receiver[16] = 0;
        receiver[17] = 0;
        receiver[18] = 0;
        receiver[19] = 0;
        //data
        for (uint16_t i = 0; i < ( data_body_length); i ++)
        {
            uint8_t value;
            if(!grbfst_exbu8(&ExOutputStorage[THR_SPI_TX].datastorage, &value))
            {
                break; 
            }
            receiver[data_start_point + i] = value;
        }
        // CRC
        ex_getCRC(&receiver[0], (length - 4), &crc);
        receiver[length - 4] = (uint8_t)(crc);
        receiver[length - 3] = (uint8_t)(crc >> 8);
        receiver[length - 2] = (uint8_t)(crc >> 16);
        receiver[length - 1] = (uint8_t)(crc >> 24);        
        break;  
    default:
        break;
    }
    return 0;

}
uint8_t getDataFromExactoDataStorage(uint8_t * receiver, const uint8_t receiver_length)
{
    if (!ExOutputStorage[THR_SPI_RX].isready)
        return 1;
    uint8_t value;
    for (uint16_t i = 0; ((grbfst_exbu8(&ExOutputStorage[THR_SPI_RX].datastorage, &value))&&(i < receiver_length)); i++)
    {
        receiver[i] = value;
    }
    return 0;
}
exactolink_package_result_t ex_checkData_ExDtStr()
{
    uint8_t value = 0;
    exactolink_package_result_t exactolink_type = EXACTOLINK_NO_DATA;
    //pack specific
    ExactoBufferUint8Type * tmp_buffer = NULL;
    *tmp_buffer = ExOutputStorage[THR_SPI_RX].datastorage;
    ex_getInfo_ExDtStr(&ExDtStr_TrasmitSPI_Info_tmp); //<===== сохраняем информацию о предыдущей итерации
    if (ExOutputStorage[THR_SPI_RX].result != THR_CTRL_WAIT)
    {
        ExDtStr_TrasmitSPI_Info.packagetype = EXACTOLINK_NO_DATA;
        setemp_exbu8(&ExOutputStorage[THR_SPI_RX].datastorage);
        return EXACTOLINK_NO_DATA;
    }
    ExOutputStorage[THR_SPI_RX].result = THR_CTRL_OK;
 
    grbfst_exbu8(tmp_buffer, &value); //[0] id
    if (value != EXACTOLINK_PCK_ID)
    {
        ExDtStr_TrasmitSPI_Info.packagetype = EXACTOLINK_NO_DATA;
        setemp_exbu8(&ExOutputStorage[THR_SPI_RX].datastorage);
        return EXACTOLINK_NO_DATA;
    }
    ExDtStr_TrasmitSPI_Info.is_data_available = 1;
    grbfst_exbu8(tmp_buffer, &value); //[1]lenL
    uint16_t pck_length;
    pck_length = (uint16_t)value;
    grbfst_exbu8(tmp_buffer, &value); //[2] lenH
    pck_length += (uint16_t)(value << 8);
    grbfst_exbu8(tmp_buffer, &value); //[3]type 
    exactolink_type = value;
    grbfst_exbu8(tmp_buffer, &value); //[4]datatype 
    if (!(( value == EXACTOLINK_START_DATA_POINT_VAL)||
         (  value == EXACTOLINK_XLXLGR_START_DATA_POINT_VAL)))
    {
        ExDtStr_TrasmitSPI_Info.packagetype = EXACTOLINK_NO_DATA;
        return EXACTOLINK_NO_DATA;
    }
    uint32_t crc_calc;
    ex_getCRC(&ExOutputStorage[THR_SPI_RX].datastorage.data[0],(pck_length - 4), &crc_calc);
    uint32_t crc_refr;
    uint16_t crc_refr_pt = pck_length - 4;
    crc_refr =  (uint32_t)(ExOutputStorage[THR_SPI_RX].datastorage.data[crc_refr_pt++]); 
    crc_refr += (uint32_t)(ExOutputStorage[THR_SPI_RX].datastorage.data[crc_refr_pt++] << 8); 
    crc_refr += (uint32_t)(ExOutputStorage[THR_SPI_RX].datastorage.data[crc_refr_pt++] << 16); 
    crc_refr += (uint32_t)(ExOutputStorage[THR_SPI_RX].datastorage.data[crc_refr_pt++] << 24); 
    if (crc_calc != crc_refr)
    {
        ExDtStr_TrasmitSPI_Info.packagetype = EXACTOLINK_CRC_ERROR;
        setemp_exbu8(&ExOutputStorage[THR_SPI_RX].datastorage);
        return EXACTOLINK_CRC_ERROR;
    }
    uint32_t exdtstr_xtspi_refcnt;
    uint16_t i;
    uint16_t frame_index;
    switch(exactolink_type)
    {
        case EXACTOLINK_LSM303AH_TYPE0:

        //     ExDtStr_TrasmitSPI_Info.length = pck_length - value - 4;
        //     ExDtStr_TrasmitSPI_Info.length_raw[0] = (uint8_t)(ExDtStr_TrasmitSPI_Info.length);
        //     ExDtStr_TrasmitSPI_Info.length_raw[1] = (uint8_t)(ExDtStr_TrasmitSPI_Info.length >> 8);

        //     ExDtStr_TrasmitSPI_Info.datatype = (uint16_t)value;
        //     grbfst_exbu8(tmp_buffer, &value); //[5] 
        //     ExDtStr_TrasmitSPI_Info.datatype += (uint16_t)(value << 8);

        //     grbfst_exbu8(tmp_buffer, &value); //[6] 
        //     ExDtStr_TrasmitSPI_Info.priority = value;

        //     grbfst_exbu8(tmp_buffer, &value); //[7] 
        //     ExDtStr_TrasmitSPI_Info.datasrc = (uint16_t)value;
        //     grbfst_exbu8(tmp_buffer, &value); //[8] 
        //     ExDtStr_TrasmitSPI_Info.datasrc += (uint16_t)(value << 8);

        //     ExDtStr_TrasmitSPI_RefCounterPrev = ExDtStr_TrasmitSPI_RefCounter;  //<=== сохраняем предудцщие данные
        //     pshfrc_exbu8(&ExOutputStorage[THR_STR_SD].datastorage, 0x11);
        //     pshfrc_exbu8(&ExOutputStorage[THR_STR_SD].datastorage, 0x11);
            
        //     exdtstr_xtspi_refcnt = 0;
        //     grbfst_exbu8(tmp_buffer, &value); //[9] 
        //     exdtstr_xtspi_refcnt = (uint32_t)value;
        //     ExDtStr_TrasmitSPI_Info.counter_raw[0] = value;
        //     pshfrc_exbu8(&ExOutputStorage[THR_STR_SD].datastorage, value);
        //     //===================================================================
        //     grbfst_exbu8(tmp_buffer, &value); //[10] 
        //     exdtstr_xtspi_refcnt += (uint32_t)(value << 8);
        //     ExDtStr_TrasmitSPI_Info.counter_raw[1] = value;
        //     pshfrc_exbu8(&ExOutputStorage[THR_STR_SD].datastorage, value);
        //     //===================================================================
        //     grbfst_exbu8(tmp_buffer, &value); //[11] 
        //     exdtstr_xtspi_refcnt += (uint32_t)(value << 16);
        //     ExDtStr_TrasmitSPI_Info.counter_raw[2] = value;
        //     pshfrc_exbu8(&ExOutputStorage[THR_STR_SD].datastorage, value);
        //     //===================================================================
        //     grbfst_exbu8(tmp_buffer, &value); //[12] 
        //     exdtstr_xtspi_refcnt += (uint32_t)(value << 24);
        //     ExDtStr_TrasmitSPI_Info.counter_raw[3] = value;
        //     pshfrc_exbu8(&ExOutputStorage[THR_STR_SD].datastorage, value);
        //     //===================================================================
        //     ExDtStr_TrasmitSPI_RefCounter = exdtstr_xtspi_refcnt;
        //     if ((ExDtStr_TrasmitSPI_RefCounter - ExDtStr_TrasmitSPI_RefCounterPrev) > 1)
        //     {
        //         // пропущены данные
        //         ExDtStr_TrasmitSPI_LostCnt++;
        //         // printk("@");
        //     }
        //     else if ((ExDtStr_TrasmitSPI_RefCounter - ExDtStr_TrasmitSPI_RefCounterPrev) == 0)
        //     {
        //         // дублирование данных
        //         // printk("#");
        //         ExDtStr_TrasmitSPI_DbleCnt++;
        //         ExDtStr_TrasmitSPI_Info.packagetype = EXACTOLINK_NO_DATA;
        //         return EXACTOLINK_NO_DATA;
        //     }
        //     ExDtStr_TrasmitSPI_Info.counter = ExDtStr_TrasmitSPI_RefCounter;

        //     frame_index = 6;
        //     ExDtStr_TransmitSPI_RxCounter++;
        //     //[13]
        //     for (i = 0; i < (ExDtStr_TrasmitSPI_Info.length); i++)
        //     {
        //         if (!grbfst_exbu8(tmp_buffer, &value))
        //         {
        //             break;
        //         }
        //         else
        //         {
        //             pshfrc_exbu8(&ExOutputStorage[THR_STR_SD].datastorage,value);
        //             frame_index++;
        //             // ex_updateCRC(value); 
        //         }
        //     }
        //     for (uint16_t i = frame_index; i < EXACTOLINK_SD_FRAME_SIZE; i++)
        //     {
        //         pshfrc_exbu8(&ExOutputStorage[THR_STR_SD].datastorage, 0x00);
        //     }
        //     EDS_SPI_pullcount += getlen_exbu8(&ExOutputStorage[THR_STR_SD].datastorage);
        //     //проверка размера пакета (тестовое) 
        // #ifdef EXACTO_DATA_STORAGE_TEST
        //     uint16_t sd_storage_data_count = getlen_exbu8(&ExOutputStorage[THR_STR_SD].datastorage);
        //     if (sd_storage_data_count % EXACTOLINK_SD_FRAME_SIZE)
        //     {
        //         //не совпадает размер с тестовым
        //         printk("@");
        //     }
        // #endif
            

        //     // ExDtStr_TransmitSPI_BuffLen = i;
        //     ExDtStr_TrasmitSPI_Info.packagetype = EXACTOLINK_LSM303AH_TYPE0;
        //     return EXACTOLINK_LSM303AH_TYPE0;
        case EXACTOLINK_SNS_XLXLGR:
            ExDtStr_TrasmitSPI_Info.length = pck_length - value - 4;
            ExDtStr_TrasmitSPI_Info.length_raw[0] = (uint8_t)(ExDtStr_TrasmitSPI_Info.length);
            ExDtStr_TrasmitSPI_Info.length_raw[1] = (uint8_t)(ExDtStr_TrasmitSPI_Info.length >> 8);

            ExDtStr_TrasmitSPI_Info.datatype = (uint16_t)value;
            grbfst_exbu8(tmp_buffer, &value); //[5] 
            ExDtStr_TrasmitSPI_Info.datatype += (uint16_t)(value << 8);

            grbfst_exbu8(tmp_buffer, &value); //[6] 
            ExDtStr_TrasmitSPI_Info.priority = value;

            grbfst_exbu8(tmp_buffer, &value); //[7] 
            ExDtStr_TrasmitSPI_Info.datasrc = (uint16_t)value;
            grbfst_exbu8(tmp_buffer, &value); //[8] 
            ExDtStr_TrasmitSPI_Info.datasrc += (uint16_t)(value << 8);

            ExDtStr_TrasmitSPI_RefCounterPrev = ExDtStr_TrasmitSPI_RefCounter;  //<=== сохраняем предудцщие данные
            // pshfrc_exbu8(&ExOutputStorage[THR_STR_SD].datastorage, 0x11);
            pshfrc_exbextu8(&ExDtStr_SD_buffer, 0x11);
            // pshfrc_exbu8(&ExOutputStorage[THR_STR_SD].datastorage, EXACTOLINK_SNS_XLXLGR);
            pshfrc_exbextu8(&ExDtStr_SD_buffer, EXACTOLINK_SNS_XLXLGR);
            
            exdtstr_xtspi_refcnt = 0;
            grbfst_exbu8(tmp_buffer, &value); //[9] 
            exdtstr_xtspi_refcnt = (uint32_t)value;
            ExDtStr_TrasmitSPI_Info.counter_raw[0] = value;
            // pshfrc_exbu8(&ExOutputStorage[THR_STR_SD].datastorage, value);
            pshfrc_exbextu8(&ExDtStr_SD_buffer, value);
            //===================================================================
            grbfst_exbu8(tmp_buffer, &value); //[10] 
            exdtstr_xtspi_refcnt += (uint32_t)(value << 8);
            ExDtStr_TrasmitSPI_Info.counter_raw[1] = value;
            // pshfrc_exbu8(&ExOutputStorage[THR_STR_SD].datastorage, value);
            pshfrc_exbextu8(&ExDtStr_SD_buffer, value);
            //===================================================================
            grbfst_exbu8(tmp_buffer, &value); //[11] 
            exdtstr_xtspi_refcnt += (uint32_t)(value << 16);
            ExDtStr_TrasmitSPI_Info.counter_raw[2] = value;
            // pshfrc_exbu8(&ExOutputStorage[THR_STR_SD].datastorage, value);
            pshfrc_exbextu8(&ExDtStr_SD_buffer, value);
            //===================================================================
            grbfst_exbu8(tmp_buffer, &value); //[12] 
            exdtstr_xtspi_refcnt += (uint32_t)(value << 24);
            ExDtStr_TrasmitSPI_Info.counter_raw[3] = value;
            // pshfrc_exbu8(&ExOutputStorage[THR_STR_SD].datastorage, value);
            pshfrc_exbextu8(&ExDtStr_SD_buffer, value);
            //===================================================================
            ExDtStr_TrasmitSPI_RefCounter = exdtstr_xtspi_refcnt;
            if ((ExDtStr_TrasmitSPI_RefCounter - ExDtStr_TrasmitSPI_RefCounterPrev) > 1)
            {
                // пропущены данные
                ExDtStr_TrasmitSPI_LostCnt++;
            }
            else if ((ExDtStr_TrasmitSPI_RefCounter - ExDtStr_TrasmitSPI_RefCounterPrev) == 0)
            {
                // дублирование данных
                ExDtStr_TrasmitSPI_DbleCnt++;
                ExDtStr_TrasmitSPI_Info.packagetype = EXACTOLINK_NO_DATA;
                return EXACTOLINK_NO_DATA;
            }
            ExDtStr_TrasmitSPI_Info.counter = ExDtStr_TrasmitSPI_RefCounter;

            ExDtStr_TransmitSPI_RxCounter++;
            grbfst_exbu8(tmp_buffer, &value); //[13] 
            grbfst_exbu8(tmp_buffer, &value); //[14] 
            grbfst_exbu8(tmp_buffer, &value); //[15] 
            grbfst_exbu8(tmp_buffer, &value); //[16] 
            grbfst_exbu8(tmp_buffer, &value); //[17] 
            grbfst_exbu8(tmp_buffer, &value); //[18] 
            grbfst_exbu8(tmp_buffer, &value); //[19] 
            //[20]
            frame_index = EXACTOLINK_SD_PT_DATA_START;
            for (i = 0; i < (ExDtStr_TrasmitSPI_Info.length); i++)
            {
                if (!grbfst_exbu8(tmp_buffer, &value))
                {
                    break;
                }
                else
                {
                    // pshfrc_exbu8(&ExOutputStorage[THR_STR_SD].datastorage,value);
                    pshfrc_exbextu8(&ExDtStr_SD_buffer, value);
                    frame_index++;
                }
            }
            for (uint16_t i = frame_index; i < EXACTOLINK_SD_FRAME_SIZE; i++)
            {
                // pshfrc_exbu8(&ExOutputStorage[THR_STR_SD].datastorage, 0x00);
                pshfrc_exbextu8(&ExDtStr_SD_buffer, value);
            }
            // EDS_SPI_pullcount += getlen_exbu8(&ExOutputStorage[THR_STR_SD].datastorage);
            EDS_SPI_pullcount += getlen_exbextu8(&ExDtStr_SD_buffer);
            ExDtStr_TrasmitSPI_Info.packagetype = EXACTOLINK_SNS_XLXLGR;
            return EXACTOLINK_SNS_XLXLGR;
        default:
            break;
    }
    return EXACTOLINK_NO_DATA;
}
uint16_t ex_pshBuf_ExDtStr(ExactoBufferUint8Type * buffer, uint16_t buffer_length, uint16_t data_type)
{
    uint8_t is_overflowed = 0;
    for (uint16_t i = 0; (i < buffer_length); i++)
    {
        uint8_t value;
        if(!grbfst_exbextu8(&ExDtStr_SD_buffer, &value))
        // if(!grbfst_exbu8(&ExOutputStorage[THR_STR_SD].datastorage, &value))
        {
            break;
        }
        if (!pshsft_exbu8(buffer, value))
        {
            if (!is_overflowed)
                is_overflowed = 1;
            //переполнение
            ExDtStr_TrasmitSPI_OverFlw++;
        }
    }
    if (is_overflowed)
        printk("qqqqqqqqqqqqqqqqqq");
    return 0;
}
uint16_t ex_getData_ExDtStr(uint8_t * buffer, uint16_t buffer_length, uint16_t data_type)
{
    for (uint16_t i = 0; (i < buffer_length); i++)
    {
        uint8_t value;
        // if (!grbfst_exbu8(&ExOutputStorage[THR_STR_SD].datastorage, &value))
        if(!grbfst_exbextu8(&ExDtStr_SD_buffer, &value))
        {
            break;
        }
        buffer[i] = value;
    }
    return 0;
}
uint8_t ex_getInfo_ExDtStr(exactolink_package_info_t * info)
{
    info->counter = ExDtStr_TrasmitSPI_Info.counter;
    info->datasrc = ExDtStr_TrasmitSPI_Info.datasrc;
    info->datatype = ExDtStr_TrasmitSPI_Info.datatype;
    info->is_data_available = ExDtStr_TrasmitSPI_Info.is_data_available;
    info->length = ExDtStr_TrasmitSPI_Info.length;
    info->packagetype = ExDtStr_TrasmitSPI_Info.packagetype;
    info->priority = ExDtStr_TrasmitSPI_Info.priority;

    info->counter_raw[0] = ExDtStr_TrasmitSPI_Info.counter_raw[0];
    info->counter_raw[1] = ExDtStr_TrasmitSPI_Info.counter_raw[1];
    info->counter_raw[2] = ExDtStr_TrasmitSPI_Info.counter_raw[2];
    info->counter_raw[3] = ExDtStr_TrasmitSPI_Info.counter_raw[3];

    info->length_raw[0] = ExDtStr_TrasmitSPI_Info.length_raw[0];
    info->length_raw[1] = ExDtStr_TrasmitSPI_Info.length_raw[1];
    return 0;
}
void ex_updateCounter_ExDtStr(thread_type_t type)
{
    switch (type)
    {
    case THR_SPI_RX:
        ExDtStr_TransmitSPI_RxCounter++; 
        break;
    case THR_SPI_TX:
        ExDtStr_TransmitSPI_TxCounter++; 
        break;
    default:
        break;
    }
}
uint32_t ex_getCounter_ExDtStr(thread_type_t type)
{
    switch (type)
    {
    case THR_SPI_RX:
        return ExDtStr_TransmitSPI_RxCounter;
    case THR_SPI_TX:
        return ExDtStr_TransmitSPI_TxCounter;
    default:
        break;
    }
    return 0;
}
uint16_t ex_getLength_ExDtStr(thread_type_t type)
{
    uint16_t value = 0;
    switch (type)
    {
    case THR_SPI_RX:
        value = getlen_exbu8(&ExOutputStorage[THR_SPI_RX].datastorage);
        break;
    case THR_SPI_TX:
        value = getlen_exbu8(&ExOutputStorage[THR_SPI_TX].datastorage);
        break;
    default:
        break;
    }

    return value;
}
uint8_t resetExactoDataStorage()
{
    lthread_launch(&ResetThread);
    if (ExSpi.isready && ! ExSpi.isenabled)
    {
        ExSpi.isenabled = 1;
        lthread_launch(&ExSpi.thread);
    }
    return 0;
}
uint8_t checkTxSender()
{
    return ExOutputStorage[THR_SPI_TX].isready;
}
uint8_t checkRxGetter()
{
    return ExOutputStorage[THR_SPI_RX].isready;
}
uint8_t setupReceiveLengthExactoDataStorage( const uint8_t length)
{
    SetupParamsThread.datalen = length;
    lthread_launch(&SetupParamsThread.thread);
    return 0;
}


