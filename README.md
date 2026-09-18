# yadpcm
Note Part 1: It seems that GitHub now uses Copilot by default for commit names/descriptions, really annoying.

Note Part 2: This is why in some prior commits about README.md have weird descriptions.... I hate GitHub...

(It took me quite a few minutes to notice it...)


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

The YADPCM audio player doesn't have looping nor volume control for now(I'm lazy).

For looping you can do something like:
```console
$ for i in {1..20}; do clear; ./yadpcm_player your_file.yadpcm; done
```

For volume control, both KDE and GNOME offer volume control per application/process.

(Tho, for GNOME, volume control per app/process is slightly hidden, and don't ask me why, I'm not a GNOME dev)


Note: Read comments in common.h for compatible targets(CPUs, compilers, OSes, etc)!




Have fun!



Copyright (c) 2026 - Yann Pierre BOYER
