#include "sd_log.h"
#include "sd_card.h"
#include <string.h>

static FATFS fs;
static FIL log_file;
static uint8_t sd_ok = 0;
static uint32_t log_count = 0;

uint8_t sd_log_init(void)
{
    if (sd_card_init() != 0) return 1;

    if (f_mount(&fs, "0:", 1) != FR_OK) return 2;

    sd_ok = 1;
    return 0;
}

uint8_t sd_log_write_csv(const char *data)
{
    if (!sd_ok) return 1;

    FRESULT res = f_open(&log_file, "0:plantlog.csv", FA_WRITE | FA_OPEN_ALWAYS);
    if (res != FR_OK) return 2;

    f_lseek(&log_file, f_size(&log_file));

    if (log_count == 0)
    {
        f_printf(&log_file, "timestamp,temp,hum,pressure,soil_moisture,rain,ai_class,confidence,alarm_level\r\n");
    }

    f_printf(&log_file, "%s\r\n", data);
    log_count++;

    f_close(&log_file);
    return 0;
}

void sd_log_flush(void)
{
    f_sync(&log_file);
}
