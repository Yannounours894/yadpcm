/*
	Copyright (c) 2026 - Yann Pierre BOYER
	Standard MIT license terms apply.
*/
/*
	YADPCM -> just IMA ADPCM with my own << custom >> file header.
	This is the YADPCM decoder as a library.
*/
#ifndef YADPCM_DEC_LIB_H
#define YADPCM_DEC_LIB_H

#include <stdint.h>
#include <stddef.h>

#include "common.h"

// 1 YADPCM Sample(Unsigned 4 bits, compressed): 1 PCM Sample(Signed 16 bits, uncompressed).
// That means it's a decompression ratio of 1:4(4 bits -> 16 bits).
// I don't think I need to explain what this function does.
static inline int16_t p_yadpcmdec_decode_yadpcm_sample_to_pcm_sample(uint8_t yadpcm_sample, YADPCMStateT* state) {
	int32_t step = step_table[state->step_idx];
	int32_t diff_q = step >> 3;

	const int v[3] = { 0x04, 0x02, 0x01 };
	for (int i = 0; i < 3; i++) {
		if (yadpcm_sample & v[i]) diff_q += step;
		step >>= 1; 
	}

	update_yadpcm_state_and_clamp(yadpcm_sample, diff_q, state);

	return (int16_t)state->pred_sample;
}

// PCM Pair: 2 PCM Samples.
// YADPCM Pair: 2 YADPCM Samples.
// I don't think I need to explain what this function does.
static inline void yadpcmdec_decode_yadpcm_pair_to_pcm_pair(uint8_t yadpcm_pair, int16_t* pcm16_s1, int16_t* pcm16_s2, YADPCMStateT* state) {
	*pcm16_s1 = p_yadpcmdec_decode_yadpcm_sample_to_pcm_sample(yadpcm_pair & 0x0F, state);
	*pcm16_s2 = p_yadpcmdec_decode_yadpcm_sample_to_pcm_sample((yadpcm_pair >> 4) & 0x0F, state);
}

// I don't think I need to explain what this function does.
static inline void yadpcmdec_decode_entire_buf(uint8_t* yadpcm_data, size_t total_samples, int16_t* out_pcm16_data) {
	YADPCMStateT state;
	state.pred_sample = 0;
	state.step_idx = 0;
	size_t yadpcm_pairs = (total_samples + 1) / 2;
	size_t samples_decoded = 0;

	for (size_t i = 0; i < yadpcm_pairs && samples_decoded < total_samples; i++) {
		int16_t pcm16_s1, pcm16_s2;
		uint8_t yadpcm_pair = yadpcm_data[i];
		yadpcmdec_decode_yadpcm_pair_to_pcm_pair(yadpcm_pair, &pcm16_s1, &pcm16_s2, &state);

		out_pcm16_data[samples_decoded++] = pcm16_s1;
		if (samples_decoded < total_samples) out_pcm16_data[samples_decoded++] = pcm16_s2;
	}
}

#endif