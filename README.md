# yadpcm
YADPCM encoder, decoder and player written in C for fun.


YADPCM is IMA ADPCM with my << custom >> file header!


AI disclosure: No AI were used for the development.


My code is licensed under the MIT license, feel free to redistribute and/or modify my code and don't forget to credit me!


common.h = Header containing common things shared between yadpcm_enc.c and yadpcm_dec_lib.h.
yadpcm_enc.c = The encoder, it outputs a .yadpcm file.
yadpcm_dec_lib.h = The actual decoder as a library.
yadpcm_dec.c = The decoder that decodes to a file.
yadpcm_player.c = A very simple YADPCM audio player using yadpcm_dec_lib.h for decoding and pulseaudio/pulseaudio-simple for playback on headphones/speakers.


Here is the ffmpeg command to convert your Mono/Stereo audio file into Mono 22.05kHz RAW PCM_S16LE:
```console
$ ffmpeg -i INPUT.EXT -c:a pcm_s16le -ar 22050 -ac 1 -f s16le OUTPUT.raw
```


Have fun!



Copyright (c) 2026 - Yann Pierre BOYER
