/*
	Copyright (c) 2026 - Yann Pierre BOYER
	Standard MIT license terms apply.
*/
/*
	YADPCM -> just IMA ADPCM with my own << custom >> file header.
	This is the YADPCM decoder that decodes YADPCM to a raw PCM_S16LE file.
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "yadpcm_dec_lib.h"

int main(int argc, char* argv[]) {
	if (argc != 2) {
		fprintf(stderr, "[FATAL ERROR] No YADPCM file found!\n");
		fprintf(stderr, "[WARNING] Only YADPCM(.yadpcm) files are supported!\n");
		fprintf(stdout, "[INFO] Usage ./yadpcm_dec audio.yadpcm\n");
		return EXIT_FAILURE;
	}

	const char* file_path = argv[1];

	FILE* yadpcm_file = fopen(file_path, "rb");
	if (!yadpcm_file) {
		fprintf(stderr, "[FATAL ERROR] Unable to open the YADPCM file!\n");
		return EXIT_FAILURE;
	}

	fseek(yadpcm_file, 0, SEEK_END);
	size_t yadpcm_buf_size = ftell(yadpcm_file);
	rewind(yadpcm_file);

	if (yadpcm_buf_size < sizeof(YADPCMHeader)) {
		fprintf(stderr, "[FATAL ERROR] File is too small to be a valid YADPCM file!\n");
		fclose(yadpcm_file);
		return EXIT_FAILURE;
	}

	uint8_t* yadpcm_buf = (uint8_t*)malloc(sizeof(uint8_t) * yadpcm_buf_size);
	if (!yadpcm_buf) {
		fprintf(stderr, "[FATAL ERROR] Mem Alloc for YADPCM Buf Failed!\n");
		fclose(yadpcm_file);
		return EXIT_FAILURE;
	}

	if (fread(yadpcm_buf, sizeof(uint8_t), yadpcm_buf_size, yadpcm_file) != yadpcm_buf_size) {
		fprintf(stderr, "[FATAL ERROR] Unable to read YADPCM file!\n");
		free(yadpcm_buf);
		fclose(yadpcm_file);
		return EXIT_FAILURE;
	}

	fclose(yadpcm_file);

	YADPCMHeader header;
	memcpy(&header, yadpcm_buf, sizeof(YADPCMHeader));
	if (strncmp(header.magic, "YADPCM", 6) != 0) {
		fprintf(stderr, "[FATAL ERROR] Invalid MAGIC number, file couldn't be identified!\n");
		free(yadpcm_buf);
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
		free(yadpcm_buf);
		return EXIT_FAILURE;
	}

	// Since I use Mono, I don't need to account for channels count.
	int16_t* pcm16_buf = (int16_t*)malloc(sizeof(int16_t) * header.total_samples);
	if (!pcm16_buf) {
		fprintf(stderr, "[FATAL ERROR] Mem Alloc for PCM_S16LE Buf Failed!\n");
		free(yadpcm_buf);
		return EXIT_FAILURE;
	}

	yadpcmdec_decode_entire_buf(yadpcm_buf + sizeof(YADPCMHeader), header.total_samples, pcm16_buf);
	free(yadpcm_buf);

	FILE* pcm16_raw_f = fopen("decoded_output.raw", "wb");
	if (!pcm16_raw_f) {
		fprintf(stderr, "[FATAL ERROR] Unable to create decoded_output.raw!\n");
		free(pcm16_buf);
		return EXIT_FAILURE;
	}

	fwrite(pcm16_buf, sizeof(int16_t) * header.total_samples, 1, pcm16_raw_f);
	fclose(pcm16_raw_f);
	free(pcm16_buf);

	return EXIT_SUCCESS;
}