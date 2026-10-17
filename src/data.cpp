#include "data.h"

PairData read_pairs(const char *filename)
{
    PairData result = {NULL, NULL, 0};
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        perror("Failed to open file");
        return result;
    }

    // ---- Pass 1: count lines ----
    int count = 0;
    float tx, ty;
    while (fscanf(fp, "%f,%f", &tx, &ty) == 2) {
        count++;
    }

    if (count == 0) {
        fclose(fp);
        return result; // empty file or invalid format
    }

    // Allocate memory
    result.x = (float *)malloc(count * sizeof(float));
    result.y = (float *)malloc(count * sizeof(float));
    if (!result.x || !result.y) {
        perror("Memory allocation failed");
        free(result.x);
        free(result.y);
        fclose(fp);
        return result;
    }
    result.count = count;

    // ---- Pass 2: rewind and read ----
    rewind(fp);
    int i = 0;
    while (i < count && fscanf(fp, "%f,%f", &result.x[i], &result.y[i]) == 2) {
        i++;
    }

    fclose(fp);
    return result;
}