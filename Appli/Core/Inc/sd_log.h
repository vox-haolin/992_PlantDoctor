#ifndef __SD_LOG_H
#define __SD_LOG_H

#include "main.h"
#include "ff.h"

uint8_t sd_log_init(void);
uint8_t sd_log_write_csv(const char *data);
void sd_log_flush(void);

#endif
