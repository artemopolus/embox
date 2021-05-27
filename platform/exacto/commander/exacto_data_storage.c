#include "exacto_data_storage.h"
#include <embox/unit.h>
// #include "blink/blinker.h"


uint32_t ExDtStr_TransmitSPI_Counter = 0;
uint32_t ExDtStr_TransmitSPI_TxCounter = 0;
uint32_t ExDtStr_TransmitSPI_RxCounter = 0;

//temporary
uint8_t ExDtStr_TrasmitSPI_Buffer[EXACTOLINK_MESSAGE_SIZE] = {0};
uint16_t ExDtStr_TransmitSPI_BuffLen = 0;
exactolink_package_info_t ExDtStr_TrasmitSPI_Info = {
    .is_data_available = 0,
    .packagetype = EXACTOLINK_NO_DATA,
};
uint32_t ExDtStr_TrasmitSPI_RefCounter = 0;
/**
 * @brief store info and data about external input
 * 
 */
exactodatastorage ExDtStorage = {
    .isEmpty = 1,
};
thread_control_t ExOutputStorage[THREAD_OUTPUT_TYPES_SZ]; 

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
    goto *lthread_resume(self, &&start);
start:
     /* инициализация */
    _trg_lthread = (thread_control_t*)self;

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
    ExOutputStorage[2].type = THR_I2C_RX;
    ExOutputStorage[3].type = THR_I2C_TX;
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
uint8_t setDataToExactoDataStorage(uint8_t * data, const uint8_t datacount, thread_control_result_t result)
{
    if (result == THR_CTRL_INIT)
    {
        clearExactoDataStorage();
        // setHeaderExactoDataStorage(1,1,64);
    }
    for (uint8_t i = 0; i < datacount; i++)
    {
        pshfrc_exbu8(&ExOutputStorage[THR_SPI_TX].datastorage, data[i]);
    }
    ExOutputStorage[THR_SPI_TX].result = result;
    return 0;
}
uint8_t getMailFromExactoDataStorage(uint8_t * receiver, const uint8_t receiver_length)
{
    if (receiver_length < getlen_exbu8(&ExOutputStorage[THR_SPI_TX].datastorage))
        return 1;
    uint8_t type = 1;
    const uint8_t pck_id = EXACTOLINK_PCK_ID;
    uint16_t address = 1;
    const uint8_t addrH = (uint8_t) (address << 8);
    const uint8_t addrL = (uint8_t) (address);
    uint16_t length = getlen_exbu8(&ExOutputStorage[THR_SPI_TX].datastorage) + EXACTOLINK_START_DATA_POINT_VAL - 2;
    const uint8_t lenH = (uint8_t) (length << 8);
    const uint8_t lenL = (uint8_t) (length);
    const uint8_t data_start_point = EXACTOLINK_START_DATA_POINT_VAL;
    //header
    receiver[0] = pck_id;
    receiver[1] = lenL;
    receiver[2] = lenH;
    receiver[3] = type;
    //
    receiver[4] = data_start_point; //datatype
    receiver[5] = 0;
    receiver[6] = 0xff;  //priority
    receiver[7] = addrL;
    receiver[8] = addrH; //datasrc

    receiver[9]  = (uint8_t) ExDtStr_TransmitSPI_TxCounter;    //counter
    receiver[10] = (uint8_t)(ExDtStr_TransmitSPI_TxCounter >> 8);
    receiver[11] = (uint8_t)(ExDtStr_TransmitSPI_TxCounter >> 16);
    receiver[12] = (uint8_t)(ExDtStr_TransmitSPI_TxCounter >> 24);

    grball_exbu8(&ExOutputStorage[THR_SPI_TX].datastorage, &receiver[EXACTOLINK_START_DATA_POINT_VAL]);
    return 0;

}
uint8_t getDataFromExactoDataStorage(uint8_t * receiver, const uint8_t receiver_length)
{
    if (!ExOutputStorage[THR_SPI_RX].isready)
        return 1;
    uint8_t value;
    for (uint8_t i = 0; ((grbfst_exbu8(&ExOutputStorage[THR_SPI_RX].datastorage, &value))&&(i < receiver_length)); i++)
    {
        receiver[i] = value;
    }
    return 0;
}
exactolink_package_result_t ex_checkData_ExDtStr()
{
    uint8_t value = 0;
    //pack specific
    ExactoBufferUint8Type * tmp_buffer = NULL;
    *tmp_buffer = ExOutputStorage[THR_SPI_RX].datastorage;
    if (!ExOutputStorage[THR_SPI_RX].isready)
    // if (!grbfst_exbu8(&ExOutputStorage[THR_SPI_RX].datastorage, &value))
    {
        ExDtStr_TrasmitSPI_Info.packagetype = EXACTOLINK_NO_DATA;
        setemp_exbu8(&ExOutputStorage[THR_SPI_RX].datastorage);
        return EXACTOLINK_NO_DATA;
    }
    grbfst_exbu8(tmp_buffer, &value); //[0] 
    // grbfst_exbu8(&ExOutputStorage[THR_SPI_RX].datastorage, &value); //id
    if (value != EXACTOLINK_PCK_ID)
    {
        ExDtStr_TrasmitSPI_Info.packagetype = EXACTOLINK_NO_DATA;
        setemp_exbu8(&ExOutputStorage[THR_SPI_RX].datastorage);
        return EXACTOLINK_NO_DATA;
    }
    ExDtStr_TrasmitSPI_Info.is_data_available = 1;
    grbfst_exbu8(tmp_buffer, &value); //[1] 
    // grbfst_exbu8(&ExOutputStorage[THR_SPI_RX].datastorage, &value); //lenL
    ExDtStr_TrasmitSPI_Info.length = (uint16_t)value;
    grbfst_exbu8(tmp_buffer, &value); //[2] 
    // grbfst_exbu8(&ExOutputStorage[THR_SPI_RX].datastorage, &value); //lenH
    ExDtStr_TrasmitSPI_Info.length += (uint16_t)(value << 8);
    grbfst_exbu8(tmp_buffer, &value); //[3] 
    //type
    grbfst_exbu8(tmp_buffer, &value); //[4] 
    //datatype
    if (value != EXACTOLINK_START_DATA_POINT_VAL)
    {
        ExDtStr_TrasmitSPI_Info.packagetype = EXACTOLINK_NO_DATA;
        return EXACTOLINK_NO_DATA;
    }
    ExDtStr_TrasmitSPI_Info.datatype = (uint16_t)value;
    grbfst_exbu8(tmp_buffer, &value); //[5] 
    ExDtStr_TrasmitSPI_Info.datatype += (uint16_t)(value << 8);
    grbfst_exbu8(tmp_buffer, &value); //[6] 
    ExDtStr_TrasmitSPI_Info.priority = value;
    grbfst_exbu8(tmp_buffer, &value); //[7] 
    ExDtStr_TrasmitSPI_Info.datasrc = (uint16_t)value;
    grbfst_exbu8(tmp_buffer, &value); //[8] 
    ExDtStr_TrasmitSPI_Info.datasrc += (uint16_t)(value << 8);
    grbfst_exbu8(tmp_buffer, &value); //[9] 
    ExDtStr_TrasmitSPI_RefCounter = (uint32_t)value;
    grbfst_exbu8(tmp_buffer, &value); //[10] 
    ExDtStr_TrasmitSPI_RefCounter += (uint32_t)(value << 8);
    grbfst_exbu8(tmp_buffer, &value); //[11] 
    ExDtStr_TrasmitSPI_RefCounter += (uint32_t)(value << 16);
    grbfst_exbu8(tmp_buffer, &value); //[12] 
    ExDtStr_TrasmitSPI_RefCounter += (uint32_t)(value << 24);
    ExDtStr_TrasmitSPI_Info.counter = ExDtStr_TrasmitSPI_RefCounter;
    ExDtStr_TransmitSPI_RxCounter++;
    uint16_t i;
    //[13]
    for (i = 0; i < (ExDtStr_TrasmitSPI_Info.length - EXACTOLINK_START_DATA_POINT_VAL); i++)
    {
        if (!grbfst_exbu8(tmp_buffer, &value))
        {
            break;
        }
        else
        {
            ExDtStr_TrasmitSPI_Buffer[i] = value;
        }
    }
    ExDtStr_TransmitSPI_BuffLen = i;
    ExDtStr_TrasmitSPI_Info.packagetype = EXACTOLINK_LSM303AH_TYPE0;
    return EXACTOLINK_LSM303AH_TYPE0;
}
uint16_t ex_getData_ExDtStr(uint8_t * buffer, uint16_t buffer_length, uint16_t data_type)
{
    for (uint16_t i = 0; i < buffer_length; i++)
    {
        buffer[i] = ExDtStr_TrasmitSPI_Buffer[i];
    }
    
    // *buffer_length = ExDtStr_TransmitSPI_BuffLen;
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


