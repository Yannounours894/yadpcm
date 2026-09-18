clang -std=c23 -O2 -march=native -Wall -Wextra -pedantic yadpcm_enc.c -o yadpcm_enc
clang -std=c23 -O2 -march=native -Wall -Wextra -pedantic yadpcm_dec.c -o yadpcm_dec
clang -std=c23 -O2 -march=native -Wall -Wextra -pedantic -lpulse -lpulse-simple yadpcm_player.c -o yadpcm_player
