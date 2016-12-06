#ifndef _MFCC_H_
#define _MFCC_H_

#define SAMPLE_RATE 16000
#define NCOEFFS 20

/* Initialize data needed for MFCC computing (allocate memory, 
 * configure threads, etc). Must be called *once* before using get_mfcc().
 * Returns 0 on success and -1 on failure. */
int init_mfcc(void);

/* Calculate Mel Frequency Cepstral Coefficients
 * in     - data to be processed (data must be normalized)
 * len    - length of data
 * coeffs - pointer to array with NCOEFFS length for storing coefficients */
void get_mfcc(double const *in, size_t len, double *coeffs);

/* Free used resources. Must be called when usage of get_mfcc() is ended. */
void free_mfcc(void);

#endif