/*
	Copyright (c) 2026 - Yann Pierre BOYER
	Standard MIT license terms apply.
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <pulse/pulseaudio.h>
#include <pulse/simple.h>

#include "yadpcm_dec_lib.h"

#define CHUNK_BYTES 1024
#define CHUNK_SAMPLES (CHUNK_BYTES * 2)

int main(int argc, char* argv[]) {
	if (argc != 2) {
		fprintf(stderr, "[FATAL ERROR] No YADPCM(.yadpcm) file found!\n");
		fprintf(stderr, "[WARNING] Only YADPCM files are supported!\n");
		fprintf(stdout, "[INFO] Usage: ./yadpcm_player audio.yadpcm\n");
		return EXIT_FAILURE;
	}

	const char* file_path = argv[1];
	FILE* yadpcm_file = fopen(file_path, "rb");
	if (!yadpcm_file) {
		fprintf(stderr, "[FATAL ERROR] Unable to open YADPCM file!\n");
		return EXIT_FAILURE;
	}

	YADPCMHeader header;
	if (fread(&header, sizeof(YADPCMHeader), 1, yadpcm_file) != 1) {
		fprintf(stderr, "[FATAL ERROR] Unable to read YADPCM Header!\n");
		fclose(yadpcm_file);
		return EXIT_FAILURE;
	}

	if (strncmp(header.magic, "YADPCM", 6) != 0) {
		fprintf(stderr, "[FATAL ERROR] Invalid MAGIC number, file couldn't be identified!\n");
		fclose(yadpcm_file);
		return EXIT_FAILURE;
	}

	// Yeah, I'm pretty dumb, size_t is UNSIGNED, why did my brain think it was SIGNED?????
	// Only supported sample rate is 22050Hz(22.05kHz) for now.
	// If a YADPCM file is corrupted, it could have a sample_rate value equal to zero.
	// Even without talking about corruption, it's a good idea to check sample_rate against supported sample rates!
	if (header.sample_rate != SAMPLE_RATE) {
		fprintf(stderr, "[FATAL ERROR] The reported Sample Rate in the YADPCM file header is incorrect!\n");
		fprintf(stderr, "[FATAL ERROR] REQUESTED: %d\n", SAMPLE_RATE);
		fprintf(stderr, "[FATAL ERROR] GOT: %zu\n", header.sample_rate);
		fprintf(stderr, "[FATAL ERROR] File may be corrupted!\n");
		return EXIT_FAILURE;
	}

	pa_simple* s;
	pa_sample_spec s_spec;
	s_spec.format = PA_SAMPLE_S16LE;
	s_spec.channels = header.channels_count;
	s_spec.rate = (uint32_t)header.sample_rate;

	s = pa_simple_new(NULL, "YADPCMPlayer", PA_STREAM_PLAYBACK, NULL, "Music", &s_spec, NULL, NULL, NULL);
	if(!s) {
		fprintf(stderr, "[FATAL ERROR] Unable to initialize PulseAudio!\n");
		fclose(yadpcm_file);
		return EXIT_FAILURE;
	}

	uint8_t enc_chunk[CHUNK_BYTES];
	int16_t pcm_chunk[CHUNK_BYTES * 2];

	YADPCMStateT state;
	state.pred_sample = 0;
	state.step_idx = 0;
	size_t samples_remaining = header.total_samples;

	size_t total_duration_in_secs = header.total_samples / header.sample_rate;
	size_t total_minutes = total_duration_in_secs / 60;
	size_t total_seconds = total_duration_in_secs % 60;

	size_t samples_processed = 0;
	while (samples_remaining > 0) {
		size_t samples_to_decode = (samples_remaining < CHUNK_SAMPLES) ? samples_remaining : CHUNK_SAMPLES;
		size_t yadpcm_pairs_to_read = (samples_to_decode + 1) / 2;

		size_t yadpcm_pairs_read = fread(enc_chunk, 1, yadpcm_pairs_to_read, yadpcm_file);
		if (yadpcm_pairs_read == 0) break;

		size_t samples_decoded = 0;
		for (size_t i = 0; i < yadpcm_pairs_read && samples_decoded < samples_to_decode; i++) {
			int16_t pcm16_s1, pcm16_s2;
			uint8_t yadpcm_pair = enc_chunk[i];
			yadpcmdec_decode_yadpcm_pair_to_pcm_pair(yadpcm_pair, &pcm16_s1, &pcm16_s2, &state);

			pcm_chunk[samples_decoded++] = pcm16_s1;
			if (samples_decoded < samples_to_decode) pcm_chunk[samples_decoded++] = pcm16_s2;
		}

		pa_simple_write(s, pcm_chunk, samples_decoded * sizeof(int16_t), NULL);
		samples_processed += samples_decoded;


		size_t elapsed_time_in_secs = samples_processed / header.sample_rate;
		size_t elapsed_minutes = elapsed_time_in_secs / 60;
		size_t elapsed_seconds = elapsed_time_in_secs % 60;
		fprintf(stdout, "\rPlayback: %02zu:%02zu/%02zu:%02zu", elapsed_minutes, elapsed_seconds, total_minutes, total_seconds);
		fflush(stdout);

		samples_remaining -= samples_decoded;
	}

	pa_simple_drain(s, NULL);
	pa_simple_free(s);
	fclose(yadpcm_file);

	return EXIT_SUCCESS;
}