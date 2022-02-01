#include "exacto_data_storage.h"
#include "hardtools/basic.h"
#include <embox/unit.h>
// #include "blink/blinker.h"
#include <kernel/printk.h>

#define EXACTO_DATA_STORAGE_TEST

uint8_t EDS_spidmairq_Marker = 0;

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
ex_thread_control_t ExDtStr_Output_Storage[THREAD_OUTPUT_TYPES_SZ];
ExactoBufferExtended ExDtStr_SD_buffer;


int16_t ExDtStr_Tmp_Str[EXDTSTR_SINGLE_DATA_STR_LENGTH] = {0};

 ex_subs_service_t ExDataStorageServices[SERVICES_COUNT];
 ex_service_info_t ExDataStorageServicesInfo = {
     .current_count = 0,
     .max_count = SERVICES_COUNT,
 };

ex_thread_control_t SetupParamsThread;


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


void startTickReactionThread( )
{
}

static int setupParamsThreadRun(struct lthread * self)
{
    ex_thread_control_t * _trg_thread = (ex_thread_control_t *) self;
    ExDtStr_Output_Storage[EX_THR_SPi_RX].datalen = _trg_thread->datalen;
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
        ExDtStr_Output_Storage[i].result = EX_THR_CTRL_NO_RESULT;
        ExDtStr_Output_Storage[i].isready = 0;
        ExDtStr_Output_Storage[i].datamaxcount = THREAD_CONTROL_BUFFER_SZ;
        setini_exbu8(&ExDtStr_Output_Storage[i].datastorage);
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
    ex_thread_control_t *_trg_lthread;
    _trg_lthread = (ex_thread_control_t*)self;
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
    _trg_lthread->result = EX_THR_CTRL_NO_RESULT;

    if (!ExDtStorage.isEmpty) 
    {
        _trg_lthread->result = EX_THR_CTRL_OK;
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
    // lthread_init(&TickReactionThread.thread, runTickReactionThread);
    ExDtStr_Output_Storage[0].type = EX_THR_SPi_RX;
    ExDtStr_Output_Storage[1].type = EX_THR_SPi_TX;
    // ExDtStr_Output_Storage[2].type = EX_THR_STR_CALC_IN;
    // ExDtStr_Output_Storage[3].type = EX_THR_STR_CALC_OUT;
    setini_exbextu8(&ExDtStr_SD_buffer);
    for (uint8_t i = 0 ; i < THREAD_OUTPUT_TYPES_SZ; i++)
    {
        ExDtStr_Output_Storage[i].result = EX_THR_CTRL_NO_RESULT;
        ExDtStr_Output_Storage[i].isready = 0;
        ExDtStr_Output_Storage[i].datamaxcount = THREAD_CONTROL_BUFFER_SZ;
        setini_exbu8(&ExDtStr_Output_Storage[i].datastorage);
    }
    for (uint8_t i = 0 ; i < ExDataStorageServicesInfo.max_count; i++)
    {
        ExDataStorageServices[0].isenabled = 0;
        ExDataStorageServices[0].type = EX_THR_NONE;
    }
    return 0;
}
/**
 * @brief инициирует легкий поток для проверки наличия данных
 * 
 * @param base легкий поток
 * @return uint8_t 
 */
uint8_t checkExactoDataStorage( ex_thread_control_t * base)
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
uint8_t initThreadExactoDataStorage( ex_thread_control_t * base )
{
    mutex_init_schedee(&base->mx);
    lthread_init(&base->thread, functionForExDtStorageHandler);
    base->result = EX_THR_CTRL_WAIT;
    return 0;
}
uint8_t transmitExactoDataStorage()
{
    // if (ExDtStr_Output_Storage[EX_THR_SPi_TX].isready)
    // {
    // ex_updateCounter_ExDtStr(EX_THR_SPi_TX);
    lthread_launch(&ExDtStr_Output_Storage[EX_THR_SPi_TX].thread);
    // }
    return 0;
}
uint8_t receiveExactoDataStorage()
{
    // if (ExDtStr_Output_Storage[EX_THR_SPi_RX].isready)
    // {
    // ex_updateCounter_ExDtStr(EX_THR_SPi_RX);
    lthread_launch(&ExDtStr_Output_Storage[EX_THR_SPi_RX].thread);
    // }
    return 0;
}
uint8_t clearExactoDataStorage()
{
    setemp_exbu8(&ExDtStr_Output_Storage[EX_THR_SPi_TX].datastorage);
    return 0;
}

void updateData2EDS(uint8_t value)
{
    // pshfrc_exbu8(&ExDtStr_Output_Storage[EX_THR_SPi_TX].datastorage, value);
    if (!pshsft_exbu8(&ExDtStr_Output_Storage[EX_THR_SPi_TX].datastorage, value))
    {
        ExDtStr_OutputSPI_OverFlw++;
    }
}
uint8_t exds_setSnsData(const uint8_t sns_id, uint8_t * data, const uint16_t datacount)
{
    if (datacount == 0)
        return 1;
    if (!exlnk_pushSnsPack(sns_id, data, datacount,&ExDtStr_Output_Storage[EX_THR_SPi_TX].datastorage))
        ExDtStr_OutputSPI_OverFlw += datacount;       
    EDS_DataStorage_UdtCnt+=datacount;
    ExDtStr_Output_Storage[EX_THR_SPi_TX].result = EX_THR_CTRL_WAIT;
    return 0;
}
uint8_t setDataToExactoDataStorage(uint8_t * data, const uint16_t datacount, ex_thread_control_result_t result)
{
    switch (result)
    {
    case EX_THR_CTRL_UNKNOWN_ERROR:
    case EX_THR_CTRL_NO_RESULT:
        return 1;
    default:
        break;
    }
    if (datacount != 0)
    {
        if(!pshsftPack_exbu8(&ExDtStr_Output_Storage[EX_THR_SPi_TX].datastorage, data, datacount))
        {
            ExDtStr_OutputSPI_OverFlw += datacount;       
        }
        EDS_DataStorage_UdtCnt+=datacount;
    }
    ExDtStr_Output_Storage[EX_THR_SPi_TX].result = result;
    
    return 0;
}
uint8_t watchPackFromExactoDataStorage(uint8_t * receiver, const uint16_t receiver_length, uint8_t type)
{
    if (type == 0)
    {
        watchsvr_exbu8(&ExDtStr_Output_Storage[EX_THR_SPi_TX].datastorage, receiver, receiver_length);
    }
    else if (type == 1)
    {
        grbsvr_exbu8(&ExDtStr_Output_Storage[EX_THR_SPi_TX].datastorage, receiver, receiver_length);
    }
    return 0;
}
uint8_t getMailFromExactoDataStorage(uint8_t * receiver, const uint16_t receiver_length)
{
    if (!ExDtStr_Output_Storage[EX_THR_SPi_TX].isready)
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
    case EXACTOLINK_SNS_XLXLGR:
        if (receiver_length <  EXACTOLINK_START_DATA_POINT_VAL + 4)
            return 1;
        data_body_length = getlen_exbu8(&ExDtStr_Output_Storage[EX_THR_SPi_TX].datastorage);
        if (data_body_length > EXACTOLINK_APOLLON_SPI_PACK_SIZE)
            data_body_length = EXACTOLINK_APOLLON_SPI_PACK_SIZE;
        data_start_point = EXACTOLINK_XLXLGR_START_DATA_POINT_VAL;
        length =  data_start_point + data_body_length + 4;
        if (length > EXACTOLINK_MESSAGE_SIZE)
        {
            length = EXACTOLINK_MESSAGE_SIZE;
            data_body_length = length - data_start_point - 4;
        }
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
        for (uint16_t i = 0; i < 4; i++)
            receiver[9+i] = (uint8_t)(ExDtStr_TransmitSPI_TxCounter >> i*8);
        
        receiver[13] = (uint8_t)(ExDtStr_OutputSPI_OverFlw );
        receiver[14] = (uint8_t)(ExDtStr_OutputSPI_OverFlw >> 8);
        receiver[15] = 0;
        receiver[16] = 0;
        receiver[17] = 0;
        receiver[18] = 0;
        receiver[19] = 0;
        //data
        for (uint16_t i = 0; ((i <  data_body_length)&&(i < (length - 4))); i ++)
        {
            uint8_t value;
            if(!grbfst_exbu8(&ExDtStr_Output_Storage[EX_THR_SPi_TX].datastorage, &value))
            {
                break; 
            }
            receiver[data_start_point + i] = value;
        }
        // CRC
        ex_getCRC(&receiver[0], (length - 4), &crc);
        for (uint16_t i = 0; i < 4; i++)
            receiver[length - 4 + i] = (uint8_t)(crc >> 8*i);

        break;  
    default:
        break;
    }
    return 0;

}
uint8_t getDataFromExactoDataStorage(uint8_t * receiver, const uint8_t receiver_length)
{
    if (!ExDtStr_Output_Storage[EX_THR_SPi_RX].isready)
        return 1;
    uint8_t value;
    for (uint16_t i = 0; ((grbfst_exbu8(&ExDtStr_Output_Storage[EX_THR_SPi_RX].datastorage, &value))&&(i < receiver_length)); i++)
    {
        receiver[i] = value;
    }
    return 0;
}
exactolink_package_result_t ex_checkData_ExDtStr()
{
    uint8_t value = 0;
    exactolink_package_result_t exactolink_type = EXACTOLINK_NO_DATA;
    ExactoBufferUint8Type * tmp_buffer = NULL;

    //void * tmp = NULL;
    for (int i = 0; i < THREAD_OUTPUT_TYPES_SZ; i++)
    {
        if (ExDtStr_Output_Storage[i].type == EX_THR_SPi_RX)
            tmp_buffer = &(ExDtStr_Output_Storage[i].datastorage);
    }
    if (tmp_buffer == 0)
        return 0;
    ex_getInfo_ExDtStr(&ExDtStr_TrasmitSPI_Info_tmp); //<===== сохраняем информацию о предыдущей итерации
    if (ExDtStr_Output_Storage[EX_THR_SPi_RX].result != EX_THR_CTRL_WAIT)
    {
        ExDtStr_TrasmitSPI_Info.packagetype = EXACTOLINK_NO_DATA;
        setemp_exbu8(&ExDtStr_Output_Storage[EX_THR_SPi_RX].datastorage);
        return EXACTOLINK_NO_DATA;
    }
    ExDtStr_Output_Storage[EX_THR_SPi_RX].result = EX_THR_CTRL_OK;
 
    grbfst_exbu8(tmp_buffer, &value); //[0] id
    if (value != EXACTOLINK_PCK_ID)
    {
        ExDtStr_TrasmitSPI_Info.packagetype = EXACTOLINK_NO_DATA;
        setemp_exbu8(&ExDtStr_Output_Storage[EX_THR_SPi_RX].datastorage);
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
    if (!(( value == EXACTOLINK_START_DATA_POINT_VAL)
            ||(value == EXACTOLINK_XLXLGR_START_DATA_POINT_VAL)
            )
        )
    {
        ExDtStr_TrasmitSPI_Info.packagetype = EXACTOLINK_NO_DATA;
        return EXACTOLINK_NO_DATA;
    }
    uint16_t pt_start_data = value;
    uint16_t cnt_data = pck_length - pt_start_data - 4;
    uint32_t crc_calc;
    ex_getCRC(&ExDtStr_Output_Storage[EX_THR_SPi_RX].datastorage.data[0],(pck_length - 4), &crc_calc);
    uint32_t crc_refr = 0;
    uint16_t crc_refr_pt = pck_length - 4;
    for (uint8_t i = 0; i < 4; i++)
        crc_refr += (uint32_t)(ExDtStr_Output_Storage[EX_THR_SPi_RX].datastorage.data[crc_refr_pt++] << i*8); 
    if (crc_calc != crc_refr)
    {
        ExDtStr_TrasmitSPI_Info.packagetype = EXACTOLINK_CRC_ERROR;
        setemp_exbu8(&ExDtStr_Output_Storage[EX_THR_SPi_RX].datastorage);
        return EXACTOLINK_NO_DATA;
    }
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

    //===================================================================
    uint32_t exdtstr_xtspi_refcnt;
    uint16_t i;
    uint16_t frame_index;
    exdtstr_xtspi_refcnt = 0;
    pshfrc_exbextu8(&ExDtStr_SD_buffer, 0x11);
    pshfrc_exbextu8(&ExDtStr_SD_buffer, exactolink_type);
    pshfrc_exbextu8(&ExDtStr_SD_buffer, (uint8_t)(cnt_data));
    pshfrc_exbextu8(&ExDtStr_SD_buffer, (uint8_t)(cnt_data >> 8));
    for (uint8_t i = 0; i < 4; i++)
    {
        grbfst_exbu8(tmp_buffer, &value); //[9] 
        exdtstr_xtspi_refcnt += (uint32_t)(value << i*8);
        ExDtStr_TrasmitSPI_Info.counter_raw[i] = value;
        pshfrc_exbextu8(&ExDtStr_SD_buffer, value);
    }
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
    //uint8_t pair[2] = {0};
    //uint16_t j = 0;
    for (i = 0; i < (ExDtStr_TrasmitSPI_Info.length); i++)
    {
        if (!grbfst_exbu8(tmp_buffer, &value))
            break;
        else
        {
            if (!pshsft_exbextu8(&ExDtStr_SD_buffer, value))
                break;
            frame_index++;
        }
    }
            
    EDS_SPI_pullcount += getlen_exbextu8(&ExDtStr_SD_buffer);
    ExDtStr_TrasmitSPI_Info.packagetype = EXACTOLINK_OK;

    return exactolink_type;
}
uint16_t ex_getRawFromSD_ExDtStr(uint8_t * trg, const uint16_t copylen)
{
    const uint16_t length = watchsvr_exbextu8(&ExDtStr_SD_buffer, trg, copylen);
    setemp_exbextu8(&ExDtStr_SD_buffer);
    return length;
}
uint8_t  ex_getRawDataStr_ExDtStr(int16_t * dst, const uint16_t dstlen)
{
    //for (uint16_t i = 0; i < EXDTSTR_SINGLE_DATA_STR_LENGTH; i++)
    // uint16_t i = 0;
    // for (i = 0; i < dstlen; i++)
    // {
    //     //dst[i] = ExDtStr_Tmp_Str[i];
    //     if (!grbfst_exbextu8(&ExDtStr_SD_buffer, &dst[i]))
    //         break;   
    // }

    return 0;
}
uint8_t ex_getInfo_ExDtStr(exactolink_package_info_t * info)
{
    // info->counter = ExDtStr_TrasmitSPI_Info.counter;
    // info->datasrc = ExDtStr_TrasmitSPI_Info.datasrc;
    // info->datatype = ExDtStr_TrasmitSPI_Info.datatype;
    // info->is_data_available = ExDtStr_TrasmitSPI_Info.is_data_available;
    // info->length = ExDtStr_TrasmitSPI_Info.length;
    // info->packagetype = ExDtStr_TrasmitSPI_Info.packagetype;
    // info->priority = ExDtStr_TrasmitSPI_Info.priority;

    // info->counter_raw[0] = ExDtStr_TrasmitSPI_Info.counter_raw[0];
    // info->counter_raw[1] = ExDtStr_TrasmitSPI_Info.counter_raw[1];
    // info->counter_raw[2] = ExDtStr_TrasmitSPI_Info.counter_raw[2];
    // info->counter_raw[3] = ExDtStr_TrasmitSPI_Info.counter_raw[3];

    // info->length_raw[0] = ExDtStr_TrasmitSPI_Info.length_raw[0];
    // info->length_raw[1] = ExDtStr_TrasmitSPI_Info.length_raw[1];
    return 0;
}
void ex_updateCounter_ExDtStr(ex_thread_type_t type)
{
    switch (type)
    {
    case EX_THR_SPi_RX:
        ExDtStr_TransmitSPI_RxCounter++; 
        break;
    case EX_THR_SPi_TX:
        ExDtStr_TransmitSPI_TxCounter++; 
        break;
    default:
        break;
    }
}
uint32_t ex_getCounter_ExDtStr(ex_thread_type_t type)
{
    switch (type)
    {
    case EX_THR_SPi_RX:
        return ExDtStr_TransmitSPI_RxCounter;
    case EX_THR_SPi_TX:
        return ExDtStr_TransmitSPI_TxCounter;
    default:
        break;
    }
    return 0;
}
uint16_t ex_getLength_ExDtStr(ex_thread_type_t type)
{
    uint16_t value = 0;
    switch (type)
    {
    case EX_THR_SPi_RX:
        value = getlen_exbu8(&ExDtStr_Output_Storage[EX_THR_SPi_RX].datastorage);
        break;
    case EX_THR_SPi_TX:
        value = getlen_exbu8(&ExDtStr_Output_Storage[EX_THR_SPi_TX].datastorage);
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
    return ExDtStr_Output_Storage[EX_THR_SPi_TX].isready;
}
uint8_t checkRxGetter()
{
    return ExDtStr_Output_Storage[EX_THR_SPi_RX].isready;
}
uint8_t setupReceiveLengthExactoDataStorage( const uint8_t length)
{
    SetupParamsThread.datalen = length;
    lthread_launch(&SetupParamsThread.thread);
    return 0;
}


