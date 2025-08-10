# WAD Music Fixer

Have you ever tried to make a music WAD, only to have it not work with some other PWADs?
Or not be compatible with Doom and Doom 2 at the same time?
This program fixes that! 

By aliasing lumps named `D_MAP01`, `D_MAP02`, ... to their names in Doom and Doom 2 (`D_E1M1`, `D_RUNNIN`, etc), this allows your music WAD to work universally.
Actual music data isn't duplicated, only new lump entries are made, and references are copied across from existing music lumps.

Note: this program will only alias a maximum of 36 WADs for Ultimate Doom, and 32 for Doom 2, as these are the amount of levels in these games.

## Usage

```wad_music_fixer [--help] <WAD file>```

## Compatability

This program has only been tested on Windows, using GCC and MinGW.
It should work in Linux too.

## License

This code is distributed under GPL-3.
