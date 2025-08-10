#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "names.h"

#define MAX_MUSIC_LUMPS 36 //We only know how to rename 36 music lumps

typedef enum WADType {
    Internal = 'I',
    Patch = 'P'
} WADType;

typedef struct Header {
    char type[4];
    uint32_t numLumps;
    uint32_t infoTableOffset; 
} Header;

typedef struct FileLump {
    uint32_t data_start;
    uint32_t size;
    char name[8];
} FileLump;

typedef struct InfoTable {
    FileLump *lumps;
    uint32_t numLumps;
} InfoTable;

void print_help(const char *prog_name) {
    printf("Usage: %s <filename>\n", prog_name);
    printf("Makes music names in a WAD compatible with Ultimate Doom, Doom 2, and ZDoom.\n");
    printf("Options:\n");
    printf("  --help    Display this help message\n");
}

InfoTable *create_music_lumps(const uint32_t *music_lumps, uint32_t music_count, FileLump *lumps, size_t num_lumps) {
    InfoTable *music_aliases = malloc(sizeof(InfoTable));
    if (!music_aliases) {
        fprintf(stderr, "Error: Memory allocation failed for music alias info table\n");
        return NULL;
    }
    FileLump *new_lumps = malloc(music_count * sizeof(FileLump) * 2); // Ultimate Doom and Doom 2 music names
    if (!new_lumps) {
        free(music_aliases);
        fprintf(stderr, "Error: Memory allocation failed for new lumps\n");
        return NULL;
    }
    music_aliases->lumps = new_lumps;
    music_aliases->numLumps = 0;

    // Alias the music lumps for Ultimate Doom and Doom 2
    bool found;
    for (uint32_t i = 0; i < music_count * 2; i++) {
        bool is_ultimate_doom = i < music_count;
        found = false;
        if (i % music_count == 0) {
            printf("\nProcessing %s music lumps...\n", is_ultimate_doom ? "Ultimate Doom" : "Doom 2");
        }
        if (i < MAX_MUSIC_LUMPS * 2 && music_lumps[i % music_count] < num_lumps) {
            // Search for Ultimate Doom and Doom 2 music names
            for (size_t j = 0; j < (is_ultimate_doom ? sizeof(ultimate_doom_music_names) / sizeof(Map) : sizeof(doom2_music_names) / sizeof(Map)); j++) {
                if (strncmp(lumps[music_lumps[i % music_count]].name, (is_ultimate_doom ? ultimate_doom_music_names : doom2_music_names)[j].key, 8) == 0) {
                    printf("Aliasing %.*s to %.*s\n", 8, lumps[music_lumps[i % music_count]].name, 8, (is_ultimate_doom ? ultimate_doom_music_names : doom2_music_names)[j].value);
                    strncpy(music_aliases->lumps[music_aliases->numLumps].name, (is_ultimate_doom ? ultimate_doom_music_names : doom2_music_names)[j].value, 8);
                    music_aliases->lumps[music_aliases->numLumps].data_start = lumps[music_lumps[i % music_count]].data_start;
                    music_aliases->lumps[music_aliases->numLumps].size = lumps[music_lumps[i % music_count]].size;
                    found = true;
                    music_aliases->numLumps++;
                    break;
                }
            }
            if (!found) {
                fprintf(stderr, "Warning: No matching %s music name found for %.*s\n", is_ultimate_doom ? "Ultimate Doom" : "Doom 2", 8, lumps[music_lumps[i % music_count]].name);
            }
        } else {
            fprintf(stderr, "Warning: Music lump index %d out of bounds, skipping.\n", i);
            continue;
        }
    }

    return music_aliases;
}

int main(int argc, char *argv[]) {
    // Check for invalid number of arguments
    if (argc < 1) {
        fprintf(stderr, "Error: No program name provided. Your shell is misbehaving.\n");
        return 1;
    }
    
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        fprintf(stderr, "Try '%s --help' for more information.\n", argv[0]);
        return 1;
    }

    // Check for help option
    if (argc == 2 && strcmp(argv[1], "--help") == 0) {
        print_help(argv[0]);
        return 0;
    }

    FILE *wadfile_h = fopen(argv[1], "rb+");
    if (!wadfile_h) {
        perror("Error opening WAD file");
        return 1;
    }

    printf("WAD File '%s' opened successfully.\n", argv[1]);

    // Read the header
    Header header;
    WADType wad_type;
    if (fread(&header, sizeof(Header), 1, wadfile_h) != 1) {
        perror("Error reading WAD header");
        fclose(wadfile_h);
        return 1;
    }

    if (strncmp(header.type, "IWAD", 4) != 0 && strncmp(header.type, "PWAD", 4) != 0) {
        perror("Error: Not a valid WAD file\n");
        fclose(wadfile_h);
        return 1;
    }
    else  {
        wad_type = (header.type[0] == 'I') ? Internal : Patch;
        printf("WAD Type: %cWAD\n", (char)wad_type);
        printf("Number of lumps: %u\n", header.numLumps);
        printf("Info table offset: %u\n", header.infoTableOffset);
    }

    // Read the lumps
    FileLump *lumps = malloc(header.numLumps * sizeof(FileLump));
    if (!lumps) {
        perror("Error: Memory allocation failed for lumps\n");
        fclose(wadfile_h);
        return 1;
    }
    if (fseek(wadfile_h, header.infoTableOffset, SEEK_SET) != 0) {
        perror("Error seeking to info table offset");
        free(lumps);
        fclose(wadfile_h);
        return 1;
    }
    size_t lumps_read = fread(lumps, sizeof(FileLump), header.numLumps, wadfile_h);
    if (lumps_read != header.numLumps) {
        perror("Error reading lumps");
        free(lumps);
        fclose(wadfile_h);
        return 1;
    }
    else {
        printf("%d Lumps read successfully:\n", lumps_read);
    }

    // Process each lump
    bool is_music = false;
    uint32_t music_count = 0;
    uint32_t music_lumps[MAX_MUSIC_LUMPS] = {0};
    for (uint32_t i = 0; i < header.numLumps; i++) {
        // Check if the lump is a music lump
        is_music = strncmp(lumps[i].name, "D_", 2) == 0;
        if (is_music) {
            music_lumps[music_count++] = i;
            if (music_count >= MAX_MUSIC_LUMPS) {
                fprintf(stderr, "Warning: More than %d music lumps found, truncating.\n", MAX_MUSIC_LUMPS);
                break;
            }
        }
        // Print the lump information
        printf("Lump %-5u [ Name: %-8.*s, Size: %10u bytes, Data Start: %10u ] %s\n",
               i, 8, lumps[i].name, lumps[i].size, lumps[i].data_start, is_music ? "(Music)" : "");
    }
    printf("Total music lumps found: %d\n", music_count);

    printf("Creating aliases for music lumps...\n");
    InfoTable *music_aliases = create_music_lumps(music_lumps, music_count, lumps, header.numLumps);
    if (!music_aliases) {
        fprintf(stderr, "Error: Failed to create new music lumps.\n");
        free(lumps);
        fclose(wadfile_h);
        return 1;
    }

    // Ask user for confirmation before appending new lumps
    printf("Found %u music lumps to append. Do you want to continue? (y/n): ", music_aliases->numLumps);
    char response;
    scanf(" %c", &response);
    if (response != 'y' && response != 'Y') {
        printf("Operation cancelled by user.\n");
        free(music_aliases->lumps);
        free(music_aliases);
        free(lumps);
        fclose(wadfile_h);
        return 0;
    }

    // Append new lumps to the end of the file
    if (fseek(wadfile_h, 0, SEEK_END) != 0) {
        perror("Error seeking to end of WAD file");
        free(music_aliases->lumps);
        free(music_aliases);
        free(lumps);
        fclose(wadfile_h);
        return 1;
    }
    fwrite(music_aliases->lumps, sizeof(FileLump), music_aliases->numLumps, wadfile_h);
    printf("Appended %u new music lumps to the WAD file.\n", music_aliases->numLumps);

    // Update the header
    header.numLumps += music_aliases->numLumps;
    fseek(wadfile_h, 0, SEEK_SET);
    if (fwrite(&header, sizeof(Header), 1, wadfile_h) != 1) {
        perror("Error writing updated header to WAD file");
        free(music_aliases->lumps);
        free(music_aliases);
        free(lumps);
        fclose(wadfile_h);
        return 1;
    }

    free(lumps);
    fclose(wadfile_h);
    return 0;
}