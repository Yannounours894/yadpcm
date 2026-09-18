/*
	Copyright (c) 2026 - Yann Pierre BOYER
	Standard MIT license terms apply.
*/
/*
	YADPCM -> just IMA ADPCM with my own << custom >> file header.
	This is the YADPCM encoder.
*/
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#include "common.h"

// 1 YADPCM Sample(Unsigned 4 bits, compressed): 1 PCM Sample(Signed 16 bits, uncompressed).
// That means it's a compression ratio of 4:1(16 bits -> 4 bits).
// I don't think I need to explain what this function does.
uint8_t encode_pcm_sample_to_yadpcm_sample(int16_t pcm_sample, YADPCMStateT* state) {
	int32_t step = step_table[state->step_idx];
	int32_t diff = (int32_t)pcm_sample - state->pred_sample;
	uint8_t yadpcm_sample = 0;

	if (diff < 0) {
		yadpcm_sample |= 0x08;
		diff = -diff;
	}

	int32_t diff_q = step >> 3;
	const int v[3] = { 0x04, 0x02, 0x01 };
	for (int i = 0; i < 3; i++) {
		if (diff >= step) {
			yadpcm_sample |= v[i];
			diff -= step;
			diff_q += step;
		}
		step >>= 1;
	}

	update_yadpcm_state_and_clamp(yadpcm_sample, diff_q, state);

	return yadpcm_sample;
}
// PCM Pair: 2 PCM Samples.
// YADPCM Pair: 2 YADPCM Samples.
// I don't think I need to explain what this function does.
uint8_t encode_pcm_pair_to_yadpcm_pair(int16_t pcm16_s1, int16_t pcm16_s2, YADPCMStateT* state) {
	uint8_t yadpcm_s1 = encode_pcm_sample_to_yadpcm_sample(pcm16_s1, state);
	uint8_t yadpcm_s2 = encode_pcm_sample_to_yadpcm_sample(pcm16_s2, state);

	return (yadpcm_s1 & 0x0F) | ((yadpcm_s2 & 0x0F) << 4); 
}

// I don't think I need to explain what this function does.
int encode_pcm_to_yadpcm(int16_t* pcm_data, size_t total_samples) {
	YADPCMHeader header;
	memcpy(header.magic, "YADPCM", 6); // MAGIC number identifying the file.
	header.sample_rate = SAMPLE_RATE; // In Hz.
	header.channels_count = 1; // Mono.
	header.bits_per_sample = 16; // Uncompressed bits per sample(Signed 16 bits).
	header.total_samples = total_samples; // Total number of uncompressed samples.

	FILE* yadpcm_file = fopen("output.yadpcm", "wb");
	if (!yadpcm_file) {
		fprintf(stderr, "[FATAL ERROR] Unable to create output.yadpcm!\n");
		return -1;
	}
	fwrite(&header, sizeof(YADPCMHeader), 1, yadpcm_file);

	size_t yadpcm_pairs = (total_samples + 1) / 2;

	uint8_t* yadpcm_buf = (uint8_t*)malloc(sizeof(uint8_t) * yadpcm_pairs);
	if (!yadpcm_buf) {
		fclose(yadpcm_file);
		fprintf(stderr, "[FATAL ERROR] Mem Alloc for YADPCM Buf Failed!\n");
		return -1;
	}

	YADPCMStateT state;
	state.pred_sample = 0;
	state.step_idx = 0;

	size_t yadpcm_pairs_idx = 0;
	for (size_t i = 0; i < total_samples; i += 2) {
		int16_t pcm16_s1 = pcm_data[i];
		int16_t pcm16_s2 = (i + 1 < total_samples) ? pcm_data[i + 1] : pcm16_s1;
		uint8_t yadpcm_pair = encode_pcm_pair_to_yadpcm_pair(pcm16_s1, pcm16_s2, &state);

		yadpcm_buf[yadpcm_pairs_idx++] = yadpcm_pair;
	}

	fwrite(yadpcm_buf, sizeof(uint8_t), yadpcm_pairs, yadpcm_file);
	fclose(yadpcm_file);
	free(yadpcm_buf);

	return 0;
}

int main(int argc, char* argv[]) {
	if (argc != 2) {
		fprintf(stderr, "[FATAL ERROR] No PCM_S16LE file found!\n");
		fprintf(stderr, "[WARNING] Only raw PCM_S16LE files are supported!\n");
		fprintf(stdout, "[INFO] Usage: ./yadpcm_enc audio_pcms16le.raw\n");
		return EXIT_FAILURE;
	}

	const char* file_path = argv[1];

	FILE* pcm16_raw_f = fopen(file_path, "rb");
	if (!pcm16_raw_f) {
		fprintf(stderr, "[FATAL ERROR] Unable to open the RAW PCM_S16LE file!\n");
		return EXIT_FAILURE;
	}

	fseek(pcm16_raw_f, 0, SEEK_END);
	size_t pcm_buf_size = ftell(pcm16_raw_f);
	rewind(pcm16_raw_f);

	size_t total_samples = pcm_buf_size / sizeof(int16_t);
	int16_t* pcm_buf = (int16_t*)malloc(sizeof(int16_t) * total_samples);
	if (!pcm_buf) {
		fclose(pcm16_raw_f);
		return EXIT_FAILURE;
	}

	if (fread(pcm_buf, sizeof(int16_t), total_samples, pcm16_raw_f) != total_samples) {
		fprintf(stderr, "[FATAL ERROR] Error while reading the RAW PCM_S16LE file!\n");
		free(pcm_buf);
		fclose(pcm16_raw_f);
		return EXIT_FAILURE;
	}

	fclose(pcm16_raw_f);

	if (encode_pcm_to_yadpcm(pcm_buf, total_samples) < 0) {
		free(pcm_buf);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}