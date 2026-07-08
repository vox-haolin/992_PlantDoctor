#ifndef __APP_AZURE_RTOS_CONFIG_H
#define __APP_AZURE_RTOS_CONFIG_H

#define TX_APP_MEM_POOL_SIZE (48 * 1024)

#if (USE_STATIC_ALLOCATION == 1)
#define TX_APP_HEAP_SIZE     (64 * 1024)
#endif

#endif
