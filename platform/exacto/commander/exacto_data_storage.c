#include "exacto_data_storage.h"
#include <embox/unit.h>
// #include "blink/blinker.h"

uint8_t * ExDt_Output_Buffer= NULL;
uint8_t   ExDt_Output_IsEnabled = 0;
uint16_t  ExDt_Output_pt = 0;

exacto_output_state_t ExDt_Output_state = EX_DIRECT;

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
        ExOutputStorage[i].state = ExDt_Output_state;
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
        lthread_launch(&ExOutputStorage[THR_SPI_TX].thread);
    // }
    return 0;
}
uint8_t receiveExactoDataStorage()
{
    // if (ExOutputStorage[THR_SPI_RX].isready)
    // {
        lthread_launch(&ExOutputStorage[THR_SPI_RX].thread);
    // }
    return 0;
}
uint8_t clearExactoDataStorage()
{
    switch ( ExDt_Output_state   )
    {
    case EX_SMPL:
        setemp_exbu8(&ExOutputStorage[THR_SPI_TX].datastorage);
        break;
    case EX_DIRECT:
        ExDt_Output_pt = 0;
        break;
    default:
        break;
    }
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
    switch ( ExDt_Output_state   )
    {
    case EX_SMPL:
        pshfrc_exbu8(&ExOutputStorage[THR_SPI_TX].datastorage, pck_id);             //[0]
        pshfrc_exbu8(&ExOutputStorage[THR_SPI_TX].datastorage, lenH);               //[1]
        pshfrc_exbu8(&ExOutputStorage[THR_SPI_TX].datastorage, lenL);               //[2]
        pshfrc_exbu8(&ExOutputStorage[THR_SPI_TX].datastorage, type);               //[3]
        pshfrc_exbu8(&ExOutputStorage[THR_SPI_TX].datastorage, data_start_point);   //[4]
        pshfrc_exbu8(&ExOutputStorage[THR_SPI_TX].datastorage, addrH);              //[5]
        pshfrc_exbu8(&ExOutputStorage[THR_SPI_TX].datastorage, addrL);              //[6]
        break;
    case EX_DIRECT:
        ExDt_Output_Buffer[0] = pck_id;
        ExDt_Output_Buffer[1] = lenH;
        ExDt_Output_Buffer[2] = lenL;
        ExDt_Output_Buffer[3] = type;
        ExDt_Output_Buffer[4] = data_start_point;
        ExDt_Output_Buffer[5] = addrH;
        ExDt_Output_Buffer[6] = addrL;
        ExDt_Output_pt = 7;
        break;
    default:
        break;
    }
}
thread_control_result_t getStateExactoDataStorage()
{
    return ExOutputStorage[THR_SPI_TX].result;
}
uint8_t setDataToExactoDataStorage(uint8_t * data, const uint8_t datacount, thread_control_result_t result)
{
    switch (ExDt_Output_state)
    {
    case EX_SMPL:
        if (result == THR_CTRL_INIT)
        {
            clearExactoDataStorage();
            setHeaderExactoDataStorage(1,1,64);
        }
        for (uint8_t i = 0; i < datacount; i++)
        {
            pshfrc_exbu8(&ExOutputStorage[THR_SPI_TX].datastorage, data[i]);
        }
        ExOutputStorage[THR_SPI_TX].result = result;
        break;
    case EX_DIRECT:
        if(ExDt_Output_IsEnabled)
        {
            for (uint8_t i = 0; (i < datacount)&&((ExDt_Output_pt + i) < EXACTO_DATA_STORAGE_SZ); i++)
            {
                ExDt_Output_Buffer[ExDt_Output_pt + i] = data[i];
            }
            ExDt_Output_pt += datacount;
        }

        break;
    default:
        break;
    }
    return 0;
}
uint8_t getMailFromExactoDataStorage(uint8_t * receiver, const uint8_t receiver_length)
{
    if (receiver_length < getlen_exbu8(&ExOutputStorage[THR_SPI_TX].datastorage))
        return 1;
    grball_exbu8(&ExOutputStorage[THR_SPI_TX].datastorage, receiver);
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


