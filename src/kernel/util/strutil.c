#include "strutil.h"

uint32_t strlen(char* str) {
    int i = 0;
    while (str[i] != 0) { i++; }
    return i;
}

void strcpy(char* src, char* dest, int count) {
    if (count == -1) {
        int i = 0;
        do {
            dest[i] = src[i];
        } while (src[i++] != 0);
    } else {
        int i;
        for (i = 0; src[i] != '\0' && i < count; i++) {
            dest[i] = src[i];
        }
        dest[i] = '\0';
    }
}

int strsplit(char* str, char c, char* dest, uint32_t skip) {
    int i = skip;
    while (str[i] != 0) {
        if (str[i] == c) {
            strcpy(str + skip, dest, i - skip);
            return i + 1;
        }
        i++;
    }
    strcpy(str + skip, dest, -1); // Copy rest of string
    return -1;
}

int strcmp(char* first, char* second, uint32_t count) {
    int i = 0;
    // If strings have differing lengths, we compare against a null byte.
    // We only stop this loop (and assert equality) if both strings terminate at the same time.
    while ((first[i] != 0 || second[i] != 0) && (count <= 0 || i < count)) {
        if (first[i] > second[i]) return 1;
        if (first[i] < second[i]) return -1;
        i++;
    }

    return 0;
}