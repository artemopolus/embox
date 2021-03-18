#include "exacto_data_storage.h"
#include <embox/unit.h>


/**
 * @brief store info and data about external input
 * 
 */
exactodatastorage ExDtStorage = {
    .isEmpty = 1,
};
thread_control_t ExOutputStorage[THREAD_OUTPUT_TYPES_SZ]; 



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
uint8_t setDataToExactoDataStorage(uint8_t * data, const uint8_t datacount)
{
    for (uint8_t i = 0; i < datacount; i++)
    {
        pshfrc_exbu8(&ExOutputStorage[THR_SPI_TX].datastorage, data[i]);
    }
    return 0;
}
uint8_t getDataFromExactoDataStorage(uint8_t * receiver, const uint8_t receiver_length)
{
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

