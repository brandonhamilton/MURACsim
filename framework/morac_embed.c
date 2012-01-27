#include <stdio.h>

//#define DEBUG 1

char * getName(const char * pathSrc)
{
    char *result;
    int result_len;

    const char *filename_begin = strrchr(pathSrc, '/');
    if (filename_begin == NULL) {
        filename_begin = pathSrc;
    } else {
        filename_begin++;
    }

    const char *filename_end = strrchr(pathSrc, '.');
    if (filename_end != NULL) {
        result_len = filename_end - filename_begin;
    } else {
        result_len = strlen(filename_begin);
    }
    result = (char *) malloc(result_len + 1);
    strncpy(result, filename_begin, result_len);
    result[result_len] = '\0';
    return result;
}

void convertCase(char *s)
{
    int i;
    for (i = 0; i < strlen(s); i++) {
         if (islower(s[i]))
              s[i] = toupper(s[i]);
    }
}

int main(int argc, char* argv[]) {

    FILE *file;
    char *buffer;
    unsigned long fileLen;
    int i;
    long words;
    int bytes;
    unsigned long index;
    const char *output_dir;

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <input>.so (output_dir)\n", argv[0]);
        return -1;
    }

    if (argc < 3) {
        output_dir = "";
    } else {
        output_dir = argv[2];
    }


#ifdef DEBUG
    printf("Opening '%s'\n", argv[1]);
#endif
    //Open file
    file = fopen(argv[1], "rb");
    if (!file) {
        fprintf(stderr, "Unable to open input file %s", argv[1]);
        return -1;
    }

#ifdef DEBUG
    printf("Getting name...");
#endif
    // Get the filename
    char *aa_name = getName(argv[1]);
    if (!aa_name) {
        fprintf(stderr, "Memory error!");
        fclose(file);
        return -1;
    }
#ifdef DEBUG
    printf("'%s'\n", aa_name);

    printf("Getting length...");
#endif
    //Get file length
    fseek(file, 0, SEEK_END);
    fileLen=ftell(file);
    fseek(file, 0, SEEK_SET);
#ifdef DEBUG
    printf("%ld\n", fileLen);
#endif
    //Allocate memory
#ifdef DEBUG
    printf("Creating buffer\n");
#endif
    buffer=(char *)malloc(fileLen+1);
    if (!buffer) {
        fprintf(stderr, "Memory error!");
        free(aa_name);
        fclose(file);
        return -1;
    }
#ifdef DEBUG
    printf("Reading file into buffer...");
#endif
    //Read file contents into buffer
    fread(buffer, fileLen, 1, file);
#ifdef DEBUG
    printf("done\n");
#endif
    fclose(file);

    char output_filename[strlen(output_dir) + 1 + strlen(aa_name) + 3];
    sprintf(output_filename, "%s/%s.h", output_dir, aa_name);

#ifdef DEBUG
    printf("Opening '%s'\n", output_filename);
#endif
    file = fopen(output_filename, "w");
    if (!file) {
        fprintf(stderr, "Unable to create output file %s", output_filename);
        free(aa_name);
        free(buffer);
        return -1;
    }
#ifdef DEBUG
    printf("Writing file header...");
#endif
    fprintf(file, "/**\n * Morac framework\n * \n * Header file for '%s'\n * Author: Brandon Hamilton <brandon.hamilton@gmail.com>\n */\n\n", aa_name);
    convertCase(aa_name);
    fprintf(file, "#ifndef %s_H\n#define %s_H\n\n", aa_name, aa_name);
    fprintf(file, "#define EXECUTE_%s int _lib_length = %d;\\\n", aa_name, fileLen);
    fprintf(file, "  asm volatile(\"mov r1,%[value]\" : : [value]\"r\"(_lib_length) : \"r1\"); \\\n");
    fprintf(file, "  asm volatile(\".align 4\\n\\t\" \\\n");
    fprintf(file, "    \".word 0xE12FFF41\\n\\t\" \\\n");
    words = fileLen / 4;
    bytes = fileLen % 4;
    index = 0;
#ifdef DEBUG
    printf("done\nWriting data %d words, %d bytes\n", words, bytes);
#endif
    for (i = 0; i < words; i++) {
        fprintf(file, "  \".word 0x%x\\n\\t\" \\\n", *(unsigned int *)&buffer[index]);
        index += 4;
    }
    unsigned int last_word = 0;
    for (i = 0; i < bytes; i++) {
        last_word |= (buffer[index++]) << 24-(i*8);
    }
    fprintf(file, "  \".word 0x%x\\n\\t\" \\\n", last_word);
    fprintf(file, "  );");
    fprintf(file, "\n#endif // %s_H\n", aa_name);

#ifdef DEBUG
    printf("Closing file\n");
#endif
    fclose(file);

    free(aa_name);
    free(buffer);

    return 0;
}
