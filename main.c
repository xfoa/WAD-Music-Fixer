#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

typedef enum WADType {
    Internal = 'I',
    Patch = 'P'
} WADType;

typedef struct Header {
    char type[4];
    uint32_t numLumps;
    uint32_t infoTableOffset; 
} Header;

typedef struct FileLump{
    uint32_t data_start;
    uint32_t size;
    char name[8];
} FileLump;

void print_help(const char *prog_name) {
    printf("Usage: %s <filename>\n", prog_name);
    printf("Makes music names in a WAD compatible with Ultimate Doom, Doom 2, and ZDoom.\n");
    printf("Options:\n");
    printf("  --help    Display this help message\n");
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

    FILE *wadfile_h = fopen(argv[1], "r+");
    if (!wadfile_h) {
        perror("Error opening file");
        return 1;
    }

    printf("WAD File '%s' opened successfully.\n", argv[1]);

    Header header;
    WADType wad_type;
    if (fread(&header, sizeof(Header), 1, wadfile_h) != 1) {
        perror("Error reading header");
        fclose(wadfile_h);
        return 1;
    }

    if (strncmp(header.type, "IWAD", 4) != 0 && strncmp(header.type, "PWAD", 4) != 0) {
        fprintf(stderr, "Error: Not a valid WAD file.\n");
        fclose(wadfile_h);
        return 1;
    }
    else  {
        wad_type = (header.type[0] == 'I') ? Internal : Patch;
        printf("WAD Type: %cWAD\n", (char)wad_type);
    }

    fclose(wadfile_h);
    return 0;
}