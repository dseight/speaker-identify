#include <stdint.h>
#include <stdlib.h>
#include <pulse/simple.h>

#define CHANNELS 1
#define RATE 16000
#define FORMAT 16
#define TIME 5

/*
 * this function record time seconds of voice
 * return value:
 *  > 0 - size of recorded data
 *  < 0 - error
 */
int record_sound(char *song_data, int time)
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

    int size = RATE * CHANNELS * FORMAT * time / 8;
    int status;

    song_data = (char *) malloc(size);

    status = pa_simple_read(pulse_desc, song_data, size, NULL);

    if (status < 0) {
        return -1; /* error reading from audio record stream */
    }

    pa_simple_free(pulse_desc);

    return size;
}

int main()
{
    char *data;
    int size;

    size = record_sound(data, TIME);
}