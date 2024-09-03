#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>
#include <Arduino.h>

double GetTime() { return (double)esp_timer_get_time() / 1000000; }

const uint32_t RAM_TYPE = MALLOC_CAP_INTERNAL;

int RamTest()
{
    initArduino();
    int rs[] = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4000};
    printf("Ram Speed Test!\n\n");
    char xx = 0;
    for (int a = 0; a < 13; a++)
    {
        printf("Read Speed 8bit ArraySize %4dkb ", rs[a]);
        int ramsize = rs[a] * 1024;
        if (RAM_TYPE == MALLOC_CAP_INTERNAL && ramsize > 256 * 1024) break;
        char *rm = (char *)heap_caps_malloc(ramsize, RAM_TYPE);

        int iters = 10; // Just enuff to boot the dog
        if (rs[a] < 512)
            iters = 50;
        double st = GetTime();
        for (int b = 0; b < iters; b++)
            for (int c = 0; c < ramsize; c++)
                xx |= rm[c];
        st = GetTime() - st;
        vTaskDelay(1); // Dog it!
        double speed = ((double)(iters * ramsize) / (1024 * 1024)) / (st);
        printf(" time: %2.1f %2.1f mb/sec  \n", st, speed);
        free(rm);
    }
    printf("\n");
    for (int a = 0; a < 13; a++)
    {
        printf("Read Speed 16bit ArraySize %4dkb ", rs[a]);
        int ramsize = rs[a] * 1024;
        if (RAM_TYPE == MALLOC_CAP_INTERNAL && ramsize > 256 * 1024) break;
        short *rm = (short *)heap_caps_malloc(ramsize, RAM_TYPE);

        int iters = 10; // Just enuff to boot the dog
        if (rs[a] < 512)
            iters = 50;
        double st = GetTime();
        for (int b = 0; b < iters; b++)
            for (int c = 0; c < ramsize / 2; c++)
                xx |= rm[c];
        st = GetTime() - st;
        vTaskDelay(1); // Dog it!
        double speed = ((double)(iters * ramsize) / (1024 * 1024)) / (st);
        printf(" time: %2.1f %2.1f mb/sec  \n", st, speed);
        free(rm);
    }
    printf("\n");
    for (int a = 0; a < 13; a++)
    {
        printf("Read Speed 32bit ArraySize %4dkb ", rs[a]);
        int ramsize = rs[a] * 1024;
        if (RAM_TYPE == MALLOC_CAP_INTERNAL && ramsize > 256 * 1024) break;
        int *rm = (int *)heap_caps_malloc(ramsize, RAM_TYPE);

        int iters = 10; // Just enuff to boot the dog
        if (rs[a] < 512)
            iters = 50;
        double st = GetTime();
        for (int b = 0; b < iters; b++)
            for (int c = 0; c < ramsize / 4; c++)
                xx |= rm[c];
        st = GetTime() - st;
        vTaskDelay(1); // Dog it!
        double speed = ((double)(iters * ramsize) / (1024 * 1024)) / (st);
        printf(" time: %2.1f %2.1f mb/sec  \n", st, speed);
        free(rm);
    }

    uint32_t fbSize = 320 * 240 * sizeof(uint16_t);
    uint16_t *fb = (uint16_t*)heap_caps_malloc(fbSize, MALLOC_CAP_INTERNAL);

    double st = GetTime();
    for (int i = 0; i < 10; i++) {
        memset(fb, (char)random(), fbSize);
    }
    double time = GetTime() - st;
    printf("\nmemset total: %f, frame: %f\n", time, time / 10);

    printf("Test done!\n");
    return xx + fb[random() % 123];
}

extern "C" void
app_main()
{
    printf("hash: %i\n", RamTest());
}