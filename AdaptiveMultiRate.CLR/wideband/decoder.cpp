/*
 *===================================================================
 *  3GPP AMR Wideband Floating-point Speech Codec
 *===================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include "typedef.h"
#include "dec_if.h"

#ifndef IF2
#include <string.h>
#define AMRWB_MAGIC_NUMBER "#!AMR-WB\n"
#endif

/*
 * DECODER.C
 *
 * Main program of the AMR WB ACELP wideband decoder.
 *
 *    Usage : decoder bitstream_file synth_file
 *
 *    Format for bitstream_file:
 *        Described in TS26.201
 *
 *    Format for synth_file:
 *      Synthesis is written to a binary file of 16 bits data.
 *
 */

extern const UWord8 block_size[];
