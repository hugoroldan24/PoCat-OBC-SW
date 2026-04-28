/* ================= INCLUDES ================= */
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include "FreeRTOS.h"
#include "semphr.h"
/* ================= MACROS AND CONSTANTS ================= */
/* ================= TYPE DEFINITIONS ================= */
/* ================= GLOBAL VARIABLES ================= */
extern SemaphoreHandle_t log_mutex;

/* ================= MODULE-LEVEL VARIABLES ================= */
/* ================= PRIVATE FUNCTION PROTOTYPES ================= */
/* ================= PUBLIC FUNCTION DEFINITIONS ================= */
void write_log(const char *level, const char *format, ...)
{
    FILE *log_file;
    va_list args;
    time_t now;
    struct tm *time_info;
    char timestamp[20];


    xSemaphoreTake(log_mutex, portMAX_DELAY);

    log_file = fopen("", "a");
    if (log_file == NULL)
    {
        return;
    }

    time(&now);
    time_info = localtime(&now);

    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", time_info);

    fprintf(log_file, "[%s] [%s] ", timestamp, level);

    va_start(args, format);
    vfprintf(log_file, format, args);
    va_end(args);

    fprintf(log_file, "\n");

    fclose(log_file);

    xSemaphoreGive(log_mutex);
}
/* ================= PRIVATE FUNCTION DEFINITIONS ================= */