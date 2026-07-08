#include "app_azure_rtos.h"
#include "app_threadx.h"
#include "main.h"

#if (USE_STATIC_ALLOCATION == 1)
__ALIGN_BEGIN static UCHAR tx_byte_pool_buffer[TX_APP_MEM_POOL_SIZE] __ALIGN_END;
static TX_BYTE_POOL tx_app_byte_pool;
#endif

VOID tx_application_define(VOID *first_unused_memory)
{
#if (USE_STATIC_ALLOCATION == 1)
    UINT status = TX_SUCCESS;
    VOID *memory_ptr;

    if (tx_byte_pool_create(&tx_app_byte_pool, "ThreadX App Memory Pool", tx_byte_pool_buffer, TX_APP_MEM_POOL_SIZE) != TX_SUCCESS)
    {
        Error_Handler();
    }

    memory_ptr = (VOID *)&tx_app_byte_pool;
    status = app_threadx_init(memory_ptr);
    if (status != TX_SUCCESS)
    {
        Error_Handler();
    }
#else
    TX_PARAMETER_NOT_USED(first_unused_memory);
#endif
}
