#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include "generate_model.h" // Include its own header for consistency

// Platform-Specific Includes
#ifdef _WIN32
    #include <windows.h>
    #include <direct.h>
    #define MKDIR(path) _mkdir(path)
#else
    #include <sys/stat.h>
    #include <dirent.h>
    #include <unistd.h>
    #define MKDIR(path) mkdir(path, 0755)
#endif

// --- "Private" data for this module ---

typedef struct {
    const char* name;
    int input_size;
    int output_size;
    int num_layers;
    const int* layer_sizes;
} ModelPreset;

static const int micro_layers[] = {8, 4};
static const int small_layers[] = {32, 16, 8};
static const int medium_layers[] = {64, 32, 32, 16};
static const int large_layers[] = {128, 64, 64, 32, 16};
static const int huge_layers[] = {512, 256, 128, 64, 32};

static ModelPreset presets[] = {
    {"Micro", 16, 4, 2, micro_layers},
    {"Small", 64, 8, 3, small_layers},
    {"Medium", 128, 16, 4, medium_layers},
    {"Large", 256, 16, 5, large_layers},
    {"Huge", 1024, 32, 5, huge_layers}
};
static const int num_presets = sizeof(presets) / sizeof(presets[0]);

static float random_float() {
    return ((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f;
}

#ifdef _WIN32
static void clear_directory(const char* path) {
    char search_path[512];
    snprintf(search_path, sizeof(search_path), "%s\\*", path);
    WIN32_FIND_DATA fd;
    HANDLE hFind = FindFirstFile(search_path, &fd);
    if (hFind != INVALID_HANDLE_VALUE) {
        printf("Clearing contents of directory '%s'...\n", path);
        do {
            if (strcmp(fd.cFileName, ".") != 0 && strcmp(fd.cFileName, "..") != 0) {
                char file_path[512];
                snprintf(file_path, sizeof(file_path), "%s\\%s", path, fd.cFileName);
                DeleteFile(file_path);
            }
        } while (FindNextFile(hFind, &fd));
        FindClose(hFind);
    }
}
#else
static void clear_directory(const char* path) {
    DIR* dir = opendir(path);
    if (dir) {
        printf("Clearing contents of directory '%s'...\n", path);
        struct dirent* entry;
        while ((entry = readdir(dir)) != NULL) {
            if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                char filepath[512];
                snprintf(filepath, sizeof(filepath), "%s/%s", path, entry->d_name);
                remove(filepath);
            }
        }
        closedir(dir);
    }
}
#endif

void run_model_generator(const char* model_dir) {
    printf("\n========================\n");
    printf("    \033[35mModel Generator\033[0m\n");
    printf("========================\n");
    printf("Please choose a model preset to generate\n");
    printf("Keep in mind that the bigger the model, the more storage it needs:\n");
    for (int i = 0; i < num_presets; i++) {
        // Calculate color gradient (green to red)
        int red = (255 * i) / (num_presets - 1);
        int green = 255 - red;
        printf("\033[38;2;%d;%d;0m", red, green);
        printf("  %d. %s (Input: %d, Output: %d, Layers: %d)\033[0m\n",
               i + 1, presets[i].name, presets[i].input_size, presets[i].output_size, presets[i].num_layers);
    }
    
    int choice = 0;
    while (choice < 1 || choice > num_presets) {
        printf("\nEnter your choice (1-%d): ", num_presets);
        if (scanf("%d", &choice) != 1) {
            while(getchar() != '\n'); // Clear invalid input
        }
    }
    const ModelPreset* preset = &presets[choice - 1];

    const char* parent_dir = "models";
    const char* default_model_dir = "models/generated_model";

    MKDIR(parent_dir);

    srand(time(NULL));
    clear_directory(default_model_dir);
    if (MKDIR(default_model_dir) != 0 && errno != EEXIST) {
        fprintf(stderr, "Error: Could not create directory '%s'.\n", default_model_dir);
        return;
    } else {
        printf("Created directory: %s\n", default_model_dir);
    }

    char filepath[256];
    snprintf(filepath, sizeof(filepath), "%s/architecture.txt", default_model_dir);
    FILE* fp = fopen(filepath, "w");
    fprintf(fp, "%d\n%d\n%d\n", preset->input_size, preset->output_size, preset->num_layers - 1);
    for (int i = 0; i < preset->num_layers; i++) fprintf(fp, "%d\n", preset->layer_sizes[i]);
    fclose(fp);
    printf("\033[32m Saved architecture.txt\033[0m\n");

    int prev_layer_size = preset->input_size;
    for (int i = 0; i < preset->num_layers; i++) {
        int current_layer_size = preset->layer_sizes[i];
        printf("  - Processing Layer %d (size: %d)\n", i, current_layer_size);
        snprintf(filepath, sizeof(filepath), "%s/layer_%d_weights.csv", default_model_dir, i);
        fp = fopen(filepath, "w");
        for (int r = 0; r < current_layer_size; r++) {
            for (int c = 0; c < prev_layer_size; c++) fprintf(fp, "%f%c", random_float(), (c == prev_layer_size - 1) ? '\0' : ',');
            fprintf(fp, "\n");
        }
        fclose(fp);
        printf("\033[32m Saved layer_%d_weights.csv\033[0m\n", i);

        snprintf(filepath, sizeof(filepath), "%s/layer_%d_biases.csv", default_model_dir, i);
        fp = fopen(filepath, "w");
        for (int b = 0; b < current_layer_size; b++) fprintf(fp, "%f%c", random_float(), (b == current_layer_size - 1) ? '\0' : ',');
        fclose(fp);
        printf("\033[32m Saved layer_%d_biases.csv\033[0m\n", i);
        prev_layer_size = current_layer_size;
    }
    printf("\nModel generation complete!\n");
}