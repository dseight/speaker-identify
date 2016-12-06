#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <pulse/simple.h>

#include "db.h"
#include "mfcc.h"

#define CHANNELS 1
#define RATE 16000
#define FORMAT 16
#define TIME 5

/*
 * this function record time seconds of voice
 * do not forget to free soung_data after usage
 * return value:
 *  > 0 - size of recorded data
 *  < 0 - error
 */
int record_sound(int16_t **song_data, int time)
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

    *song_data = (int16_t *) malloc(size);
    if (*song_data == NULL) {
        pa_simple_free(pulse_desc);
        return -1;
    }

    status = pa_simple_read(pulse_desc, *song_data, size, NULL);

    if (status < 0) {
        pa_simple_free(pulse_desc);
        return -1; /* error reading from audio record stream */
    }

    pa_simple_free(pulse_desc);

    return size;
}

int main()
{
    int16_t *data = NULL;
    double mfcc[NCOEFFS];

    if (init_mfcc() == -1) {
        fprintf(stderr, "Error: MFCC initialization failed\n");
        return 1;
    }

    printf("MFCC initializaton completed\n");

    int size = record_sound(&data, TIME);
    if (size < 0) {
        fprintf(stderr, "Error: sound recording failed\n");
        free_mfcc();
        free(data);
        return 1;
    }

    size /= sizeof(int16_t);

    printf("Sound recorded\n");

    double *normalized_data = (double *) malloc(size * sizeof(double));
    if (normalized_data == NULL) {
        fprintf(stderr, "Error: memory allocation failed\n");
        free(data);
        return 1;
    }

    double max = 0.0;
    for (int i = 0; i < size; ++i)
        if (abs(data[i]) > max)
            max = abs(data[i]);

    for (int i = 0; i < size; ++i)
        normalized_data[i] = (double) data[i] / max;

    printf("Normalization completed\n");

    get_mfcc(normalized_data, size, mfcc);
    printf("MFCC computed\n");

    update_database(mfcc);
    printf("Database updated\n");

    //determine_name(mfcc);
    //printf("Name determined\n");

    free_mfcc();
    free(data);
    free(normalized_data);
    
    return 0;
}
