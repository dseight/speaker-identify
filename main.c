#include <stdint.h>
#include <stdlib.h>
#include <pulse/simple.h>

#define CHANNELS 1
#define RATE 16000
#define FORMAT 16
#define TIME 5

int main()
{
    pa_simple *pulse_desc;
    pa_sample_spec parametrs;

    /* pulseaudio parametrs */
    parametrs.format = PA_SAMPLE_S16NE;
    parametrs.channels = CHANNELS;
    parametrs.rate = RATE;

    pulse_desc = pa_simple_new(
        NULL,
        "",
        PA_STREAM_RECORD,
        NULL,
        "",
        &parametrs,
        NULL,
        NULL,
        NULL);

    if (pulse_desc == NULL) {
        return -1; /* error opening audio record stream */
    }

    int size = RATE * CHANNELS * FORMAT * TIME / 8;
    int status;
    char *song_data;

    song_data = (char *) malloc(size);

    status = pa_simple_read(pulse_desc, song_data, size, NULL);

    if (status < 0) {
        return -1; /* error reading from audio record stream */
    }

    pa_simple_free(pulse_desc);

    //передавать дальше следует указатель на массив такого типа
    //(int16_t *) song_data;
    //и его размер (и не забыть его освободить)
}