/*
	Copyright (c) 2026 - Yann Pierre BOYER
	Standard MIT license terms apply.
*/
/*
	YADPCM -> just IMA ADPCM with my own << custom >> file header.
	This is for shared/common things which I use in the encoder and decoder.
*/
#ifndef YADPCM_COMMON_H
#define YADPCM_COMMON_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define IDX_TBL_SIZE 16
#define STP_TBL_SIZE 89
#define SAMPLE_RATE 22050 // Only supported sample rate as of now.

static const int index_table[IDX_TBL_SIZE] = {
    -1, -1, -1, -1, 2, 4, 6, 8,
    -1, -1, -1, -1, 2, 4, 6, 8
};

static const int step_table[STP_TBL_SIZE] = {
       7,     8,     9,    10,    11,    12,    13,    14,    16,    17,
      19,    21,    23,    25,    28,    31,    34,    37,    41,    45,
      50,    55,    60,    66,    73,    80,    88,    97,   107,   118,
     130,   143,   157,   173,   190,   209,   230,   253,   279,   307,
     337,   371,   408,   449,   494,   544,   598,   658,   724,   796,
     876,   963,  1060,  1166,  1282,  1411,  1552,  1707,  1878,  2066,
    2272,  2499,  2749,  3024,  3327,  3660,  4026,  4428,  4871,  5358,
    5894,  6484,  7132,  7845,  8630,  9493, 10442, 11487, 12635, 13899,
   15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767
};

/*
	I know that using size_t isn't really recommended for a file header.
	Since I only target 64bit Linux and CPUs I'm okay with using size_t.
	As for Endiannes, most CPUs are Little-Endian, I know some are still Big-Endian.
	The CPUs I target are all Little-Endian, therefore I'm doing the bare minimum.

	Targeted CPU ISAs: x86_64, ARM64(not tested yet), RISC-V 64bit(not tested and never will be, because I don't own RISC-V hardware), no 32bit ISAs.
	Targeted Endianness: Little-Endian.
	Targeted CPUs: Any Intel/AMD CPU made in the last 20 years(2006 -> today), any ARMv8/ARMv9 CPU.
	Targeted OS: GNU/Linux(tested on my personal AMD Ryzen Zen 4 laptop with Fedora 44 Workstation)
	Targeted Compilers: Clang(the compiler I'm personally using), GCC.

	A quick note on targeted CPUs and targeted OS:
		IMA ADPCM encoding and decoding is extremely light on the CPU.
		Especially decoding, it can run on extremely crappy CPUs.
		So why my encoder, decoder and player all require 64bit CPUs and 64bit GNU/Linux?
		I'm lazy and it's just a fun project, I'm not trying to make it run on everything possible.
		The issue isn't even my code performance! Different ISAs, OSes and even Endianness all make the code more complex.
		By targeting only << modern >> CPUs, ISAs and OS I'm avoiding the headache that comes with supporting everything under the sun.
*/
#pragma pack(push, 1)
typedef struct {
	char magic[6]; // Y A D P C M
	size_t sample_rate; // 22050Hz
	size_t channels_count; // 1 because it only supports Mono for now.
	size_t bits_per_sample; // 16 bits per sample(uncompressed).
	size_t total_samples; // Total number of uncompressed samples.
} YADPCMHeader;
#pragma pack(pop)

/*
	magic = 6 bytes, sample_rate = 8 bytes, chan_count = 8 bytes, bits_per_sample = 8 bytes, total_samples = 8 bytes.
	6 + 8 + 8 + 8 + 8 = 38
*/ 
static_assert(sizeof(YADPCMHeader) == 38, "YADPCMHeader must be 38 bytes, 64bit target required!\n");

typedef struct {
	int32_t pred_sample;
	int8_t step_idx;
} YADPCMStateT;

// I don't think I need to explain what this function does.
static inline void update_yadpcm_state_and_clamp(uint8_t yadpcm_sample, int32_t diff_q, YADPCMStateT* state) {
	// Update the YADPCM State.
	if (yadpcm_sample & 0x08) state->pred_sample -= diff_q;
	else state->pred_sample += diff_q;
	state->step_idx += index_table[yadpcm_sample];

	// Clamp.
	if (state->pred_sample > 32767) state->pred_sample = 32767;
	else if (state->pred_sample < -32768) state->pred_sample = -32768;
	if (state->step_idx < 0) state->step_idx = 0;
	else if (state->step_idx > 88) state->step_idx = 88;
}

#endif