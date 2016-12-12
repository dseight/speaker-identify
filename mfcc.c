#include <stdio.h>
#include <complex.h>
#include <math.h>
#include <string.h>
#include <fftw3.h>
#include "mfcc.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950288
#endif

#define WINDOW_SIZE 2048
#define TIME_PERIOD ((double) WINDOW_SIZE / (double) SAMPLE_RATE)

#define LOPASS 130
#define HIPASS 6800     // Must not be higher than half of sample rate

/* Filterbank from Mel Frequency Cepstrum Coefficients.
 * Count of filterbank coefficients equals WINDOW_SIZE / 2 because
 * only the first half of frequencies is sufficient. */
static double filterbank[NCOEFFS][WINDOW_SIZE];

/* Frequencies indices for filterbank computing */
static int filter_unaligned[NCOEFFS + 2];
static int *filter = filter_unaligned + 1;

/* Hamming window */
static double hamming[WINDOW_SIZE];

static fftw_complex *in;
static fftw_complex *out;
static fftw_plan p;


/* Convert frequency f from Hz to frequency index */
static inline int hz_to_index(double f)
{
    return (int) f * TIME_PERIOD;
}

/* Convert frequency f from Hz to Mel scale */
static inline double hz_to_mel(double f)
{
    return 1127.0 * log(1 + f / 700.0);
}

/* Convert frequency f from Mel to Hz scale */
static inline double mel_to_hz(double f)
{
    return 700.0 * (exp(f / 1127.0) - 1);
}

/* Calculate squared amplitudes of Fourier transfrom on specified data */
static void fourier_amps(double *data)
{
    // Apply Hamming window to data before processing
    for (int i = 0; i < WINDOW_SIZE; ++i)
        in[i] = hamming[i] * data[i] + 1 * I;

    fftw_execute_dft(p, in, out);

    for (int i = 0; i < WINDOW_SIZE; ++i) {
        data[i] = cabs(out[i]);
        data[i] *= data[i];
    }
}

/* Calculate Mel Frequency Cepstral Coefficients
 * in     - data to be processed
 * len    - length of data
 * coeffs - pointer to array with NCOEFFS length for storing coefficients */
void get_mfcc(double const *in, size_t len, double *coeffs)
{
    // TODO: add multithreading
    double window[WINDOW_SIZE];
    double energy[NCOEFFS];

    // Initialize all coefficients
    for (int i = 0; i < NCOEFFS; ++i)
        coeffs[i] = 0.0;

    // Go through all intersecting windows
    for (size_t i = 0; i + WINDOW_SIZE < len; i += WINDOW_SIZE / 2) {
        memcpy(window, in + i, WINDOW_SIZE * sizeof(double));
        fourier_amps(window);

        // Calculate energy for each window
        for (int m = 0; m < NCOEFFS; ++m) {
            energy[m] = 0.0;

            for (int k = 0; k < WINDOW_SIZE; ++k)
                energy[m] += window[k] * filterbank[m][k];

            energy[m] = log(energy[m]);
        }

        // Apply discrete cosine transfrom to get coefficients
        for (int n = 0; n < NCOEFFS; ++n) {
            for (int m = 0; m < NCOEFFS; ++m)
                coeffs[n] += energy[m]
                    * cos(M_PI * n * (m + 1.0/2.0) / (double) NCOEFFS);
        }
    }

    // Get average value of coefficients
    for (int i = 0; i < NCOEFFS; ++i)
        coeffs[i] /= (double) NCOEFFS;
}

/* Initialize data needed for MFCC computing (allocate memory, 
 * configure threads, etc). Must be called *once* before using get_mfcc().
 * Returns 0 on success and -1 on failure. */
int init_mfcc(void)
{
    in = (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * WINDOW_SIZE);

    if (in == NULL)
        return -1;

    out = (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * WINDOW_SIZE);

    if (out == NULL) {
        fftw_free(in);
        return -1;
    }

    // Tell FFTW to use sub-optimal plan and allow it to destroy input data
    p = fftw_plan_dft_1d(WINDOW_SIZE, in, out, FFTW_FORWARD, 
        FFTW_ESTIMATE | FFTW_DESTROY_INPUT);

    // Calculate Hamming window
    for (int i = 0; i < WINDOW_SIZE; ++i)
        hamming[i] = 0.53836 
                   - 0.46164 * cos(2 * M_PI * i / (double) (WINDOW_SIZE - 1));

    // Calculate frequencies for filterbank computing
    double step = (hz_to_mel(HIPASS) - hz_to_mel(LOPASS)) / (NCOEFFS + 1);
    for (int i = 0; i < NCOEFFS + 2; ++i)
        filter_unaligned[i] = hz_to_index(
            mel_to_hz(hz_to_mel(LOPASS) + i * step)
        );

    // Calculate filterbank
    for (int m = 0; m < NCOEFFS; ++m) {
        for (int k = 0; k < WINDOW_SIZE; ++k) {
            if (k < filter[m - 1]) {
                filterbank[m][k] = 0.0;
            } else if (filter[m - 1] <= k && k < filter[m]) {
                filterbank[m][k] =
                    (k - filter[m - 1]) / (double) (filter[m] - filter[m - 1]);
            } else if (filter[m] <= k && k <= filter[m + 1]) {
                filterbank[m][k] =
                    (filter[m + 1] - k) / (double) (filter[m + 1] - filter[m]);
            } else {
                filterbank[m][k] = 0.0;
            }
        }
    }

    return 0;
}

/* Must be called when usage of get_mfcc() is ended */
void free_mfcc(void)
{
    fftw_destroy_plan(p);
    fftw_free(in);
    fftw_free(out);
}
