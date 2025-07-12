#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "model.h"
#include "generate_model.h"
#include "model_manager.h"

#ifdef _WIN32
#include <windows.h>
void enable_virtual_terminal_processing() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) return;
    DWORD dwMode = 0;
    if (!GetConsoleMode(hOut, &dwMode)) return;
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    if (!SetConsoleMode(hOut, dwMode)) return;
}
#endif

static int load_input_from_file(const char* filepath, float* input_array, int expected_size) {
    FILE* fp = fopen(filepath, "r");
    if (!fp) {
        fprintf(stderr, "\033[31mERROR: Could not open input file '%s'.\033[0m\n", filepath);
        return 0;
    }
    int i = 0;
    while (i < expected_size && fscanf(fp, "%f,", &input_array[i]) == 1) {
        i++;
    }
    if (i < expected_size) {
        fprintf(stderr, "\033[33mWARNING: File '%s' contained fewer elements than expected (%d/%d). Remaining values will be set to 0.\033[0m\n", filepath, i, expected_size);
        for (; i < expected_size; i++) {
            input_array[i] = 0.0f;
        }
    }
    fclose(fp);
    return 1;
}

static int list_csv_files(const char* dir, char*** out_files) {
    int count = 0, cap = 8;
    char** files = malloc(sizeof(char*) * cap);
    if (!files) return 0;

    #ifdef _WIN32
    char search_path[512];
    snprintf(search_path, sizeof(search_path), "%s\\*.csv", dir);
    WIN32_FIND_DATAA fd;
    HANDLE hFind = FindFirstFileA(search_path, &fd);
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                if (count == cap) {
                    cap *= 2;
                    char** tmp = realloc(files, sizeof(char*) * cap);
                    if (!tmp) break;
                    files = tmp;
                }
                char path[512];
                snprintf(path, sizeof(path), "%s\\%s", dir, fd.cFileName);
                files[count++] = strdup(path);
            }
        } while (FindNextFileA(hFind, &fd));
        FindClose(hFind);
    }
    #else
    DIR* d = opendir(dir);
    if (d) {
        struct dirent* entry;
        while ((entry = readdir(d))) {
            size_t len = strlen(entry->d_name);
            if (len > 4 && strcmp(entry->d_name + len - 4, ".csv") == 0) {
                if (count == cap) {
                    cap *= 2;
                    char** tmp = realloc(files, sizeof(char*) * cap);
                    if (!tmp) break;
                    files = tmp;
                }
                char path[512];
                snprintf(path, sizeof(path), "%s/%s", dir, entry->d_name);
                files[count++] = strdup(path);
            }
        }
        closedir(d);
    }
    #endif

    *out_files = files;
    return count;
}

// Inference Runner
static void run_inference() {
    printf("\n\033[36m--- Running Inference ---\033[0m\n");
    
    DiscoveredModel* models = NULL;
    int model_count = discover_models(&models);

    if (model_count == 0) {
        fprintf(stderr, "\033[31mNo models found. Please generate or import a model first.\033[0m\n");
        return;
    }

    const char* rainbow[] = {"\033[31m", "\033[33m", "\033[32m", "\033[36m", "\033[34m", "\033[35m"};
    int num_colors = sizeof(rainbow) / sizeof(rainbow[0]);

    printf("Please select a model to run:\n");
    for (int i = 0; i < model_count; i++) {
        printf("%s  %d. %s\033[0m\n", rainbow[i % num_colors], i + 1, models[i].path);
    }
    
    int choice = 0;
    while (choice < 1 || choice > model_count) {
        printf("Enter your choice (1-%d): ", model_count);
        if (scanf("%d", &choice) != 1) while(getchar() != '\n');
    }
    const char* model_path = models[choice - 1].path;

    TinyNN_Model* model = create_model_from_path(model_path);
    if (!model) {
        fprintf(stderr, "\033[31mFailed to load model from '%s'.\033[0m\n", model_path);
        free(models);
        return;
    }
    printf("\033[32mModel loaded successfully\033[0m (Input: %d, Output: %d).\n", model->input_size, model->output_size);
    free(models); // Free the list of models now that we've chosen one

    float* input = (float*)malloc(sizeof(float) * model->input_size);
    if (!input) { free_model(model); return; }

    printf("\nChoose input data source:\n");
    printf("  1. Use dummy data (all 1.0s)\n");
    printf("  2. Load from a CSV file in '\033[33mdata/\033[0m'\n");
    int input_choice = 0;
    while (input_choice < 1 || input_choice > 2) {
        printf("Enter your choice (1-2): ");
        if (scanf("%d", &input_choice) != 1) while(getchar() != '\n');
    }

    switch (input_choice) {
        case 1:
            printf("\033[34mUsing dummy data...\033[0m\n");
            for (int i = 0; i < model->input_size; i++) input[i] = 1.0f;
            break;
        case 2: {
            // List CSV files
            char** csv_files = NULL;
            int csv_count = list_csv_files("data", &csv_files);
            if (csv_count == 0) {
                printf("\033[31mNo CSV files found in 'data/'.\033[0m\n");
                free(input);
                free_model(model);
                return;
            }
            const char* rainbow[] = {"\033[31m", "\033[33m", "\033[32m", "\033[36m", "\033[34m", "\033[35m"};
            int num_colors = sizeof(rainbow) / sizeof(rainbow[0]);
            printf("Select an input CSV file:\n");
            for (int i = 0; i < csv_count; i++) {
                printf("%s  %d. %s\033[0m\n", rainbow[i % num_colors], i + 1, csv_files[i]);
            }
            int file_choice = 0;
            while (file_choice < 1 || file_choice > csv_count) {
                printf("Enter your choice (1-%d): ", csv_count);
                if (scanf("%d", &file_choice) != 1) while(getchar() != '\n');
            }
            printf("\033[34mLoading data from '%s'...\033[0m\n", csv_files[file_choice - 1]);
            if (!load_input_from_file(csv_files[file_choice - 1], input, model->input_size)) {
                for (int i = 0; i < csv_count; i++) free(csv_files[i]);
                free(csv_files);
                free(input);
                free_model(model);
                return;
            }
            for (int i = 0; i < csv_count; i++) free(csv_files[i]);
            free(csv_files);
            break;
        }
    }

    printf("\033[36mRunning forward pass...\033[0m\n");
    float* output = forward_pass(model, input);

    printf("\n\033[35m--- Prediction Results ---\033[0m\n");
    float sum = 0.0f;
    for (int i = 0; i < model->output_size; i++) {
        float val = output[i];
        int red = (int)((1.0f - val) * 255);
        int green = (int)(val * 255);
        printf("  Class %d: \033[38;2;%d;%d;0m\t%.6f\033[0m\n", i, red, green, val);
        sum += val;
    }
    printf("--------------------------\n");
    printf("Sum of probabilities: %.6f\n", sum);

    free(input);
    free(output);
    free_model(model);
    printf("\n\033[32mInference complete and memory freed.\033[0m\n");
}

static void print_main_menu() {
    printf("\n========================\n");
    printf("    \033[35mTinyNN Main Menu\033[0m\n");
    printf("========================\n");
    printf("  1. \033[36mGenerate a New Model\033[0m\n");
    printf("  2. \033[32mRun Inference on a Model\033[0m\n");
    printf("  3. \033[33mImport External Model\033[0m\n");
    printf("  0. \033[31mExit\033[0m\n");
    printf("------------------------\n");
}

int main() {
    #ifdef _WIN32
    enable_virtual_terminal_processing();
    #endif

    int choice = -1;

    while (choice != 0) {
        print_main_menu();
        printf("Enter your choice: ");
        if (scanf("%d", &choice) != 1) {
            printf("Invalid input. Please enter a number.\n");
            while(getchar() != '\n');
            choice = -1;
            continue;
        }

        switch (choice) {
            case 1:
                run_model_generator();
                break;
            case 2:
                run_inference();
                break;
            case 3:
                run_model_importer();
                break;
            case 0:
                printf("Exiting. Goodbye!\n");
                break;
            default:
                printf("Invalid choice. Please try again.\n");
                break;
        }
    }

    return 0;
}