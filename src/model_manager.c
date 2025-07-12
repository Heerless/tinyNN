#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "model_manager.h"

#ifdef _WIN32
    #include <windows.h>
    #include <direct.h>
    #define MKDIR(path) _mkdir(path)
    #define PATH_SEPARATOR '\\'
#else
    #include <sys/stat.h>
    #include <dirent.h>
    #include <unistd.h>
    #define MKDIR(path) mkdir(path, 0755)
    #define PATH_SEPARATOR '/'
#endif

static int copy_file(const char* src, const char* dest) {
    FILE *src_fp, *dest_fp;
    char buffer[4096];
    size_t n;
    src_fp = fopen(src, "rb");
    if (!src_fp) return 0;
    dest_fp = fopen(dest, "wb");
    if (!dest_fp) { fclose(src_fp); return 0; }
    while ((n = fread(buffer, 1, sizeof(buffer), src_fp)) > 0) {
        if (fwrite(buffer, 1, n, dest_fp) != n) {
            fclose(src_fp);
            fclose(dest_fp);
            return 0;
        }
    }
    fclose(src_fp);
    fclose(dest_fp);
    return 1;
}

static int safe_path_join(char* dest, size_t dest_size, const char* base, const char* component) {
    int written = snprintf(dest, dest_size, "%s%c%s", base, PATH_SEPARATOR, component);
    if (written < 0 || (size_t)written >= dest_size) {
        fprintf(stderr, "Error: Path concatenation would result in a buffer overflow.\n");
        return 0;
    }
    return 1;
}

static int is_valid_model_dir(const char* path) {
    char arch_path[SAFE_PATH_MAX];
    if (!safe_path_join(arch_path, sizeof(arch_path), path, "architecture.txt")) {
        return 0;
    }
    FILE* fp = fopen(arch_path, "r");
    if (fp) {
        fclose(fp);
        return 1;
    }
    return 0;
}

static void add_model_to_list(const char* path, DiscoveredModel** models_out, int* count, int* capacity) {
    if (*count >= *capacity) {
        *capacity *= 2;
        *models_out = realloc(*models_out, (*capacity) * sizeof(DiscoveredModel));
    }
    snprintf((*models_out)[*count].path, SAFE_PATH_MAX, "%s", path);
    (*count)++;
}

void run_model_importer() {
    char src_path[SAFE_PATH_MAX];
    char new_name[128];

    printf("\n--- Import External Model ---\n");
    printf("This tool will validate and copy a model folder (exported to our .csv format)\n");
    printf("into a managed '\033[33mmodels/\033[0m' directory.\n");

    printf("\nEnter the path to the source model directory: ");
    scanf("%4095s", src_path);

    if (!is_valid_model_dir(src_path)) { return; }

    printf("Enter a new name for this model (no spaces): ");
    scanf("%127s", new_name);

    const char* parent_dir = "models";
    MKDIR(parent_dir);

    char dest_path[SAFE_PATH_MAX];
    snprintf(dest_path, sizeof(dest_path), "%s/%s", parent_dir, new_name);

    if (MKDIR(dest_path) != 0) { /* ... warning ... */ }

    printf("Copying model files...\n");
    #ifdef _WIN32
        char search_path[SAFE_PATH_MAX];
        if (strlen(src_path) + strlen("\\*") >= SAFE_PATH_MAX) {
            fprintf(stderr, "Error: Source path is too long to process.\n");
            return;
        }
        snprintf(search_path, sizeof(search_path), "%s\\*", src_path);

        WIN32_FIND_DATA fd;
        HANDLE hFind = FindFirstFile(search_path, &fd);
        if (hFind != INVALID_HANDLE_VALUE) {
            do {
                if (strcmp(fd.cFileName, ".") != 0 && strcmp(fd.cFileName, "..") != 0) {
                    char src_file[SAFE_PATH_MAX], dest_file[SAFE_PATH_MAX];
                    if (safe_path_join(src_file, sizeof(src_file), src_path, fd.cFileName) &&
                        safe_path_join(dest_file, sizeof(dest_file), dest_path, fd.cFileName)) {
                        if (!copy_file(src_file, dest_file)) {
                             fprintf(stderr, "\033[31mFailed to copy %s\033[0m\n", fd.cFileName);
                        }
                    }
                }
            } while (FindNextFile(hFind, &fd));
            FindClose(hFind);
        }
    #else // POSIX
        DIR* dir = opendir(src_path);
        if (dir) {
            struct dirent* entry;
            while ((entry = readdir(dir)) != NULL) {
                 if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                    char src_file[SAFE_PATH_MAX], dest_file[SAFE_PATH_MAX];
                    if (safe_path_join(src_file, sizeof(src_file), src_path, entry->d_name) &&
                        safe_path_join(dest_file, sizeof(dest_file), dest_path, entry->d_name)) {
                        if (!copy_file(src_file, dest_file)) {
                             fprintf(stderr, "\033[31mFailed to copy %s\033[0m\n", entry->d_name);
                        }
                    }
                 }
            }
            closedir(dir);
        }
    #endif

    printf("\033[32mModel '%s' imported successfully!\033[0m\n", new_name);
}

int discover_models(DiscoveredModel** models_out) {
    int count = 0;
    int capacity = 4;
    *models_out = malloc(capacity * sizeof(DiscoveredModel));
    if (!*models_out) return 0;

    const char* models_dir_path = "models";

    #ifdef _WIN32
        char search_path[SAFE_PATH_MAX];
        if (strlen(models_dir_path) + strlen("\\*") >= SAFE_PATH_MAX) return 0;
        snprintf(search_path, sizeof(search_path), "%s\\*", models_dir_path);
        WIN32_FIND_DATA fd;
        HANDLE hFind = FindFirstFile(search_path, &fd);
        if (hFind != INVALID_HANDLE_VALUE) {
            do {
                if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && strcmp(fd.cFileName, ".") != 0 && strcmp(fd.cFileName, "..") != 0) {
                    char model_path[SAFE_PATH_MAX];
                    if (safe_path_join(model_path, sizeof(model_path), models_dir_path, fd.cFileName)) {
                        if (is_valid_model_dir(model_path)) {
                            add_model_to_list(model_path, models_out, &count, &capacity);
                        }
                    }
                }
            } while (FindNextFile(hFind, &fd));
            FindClose(hFind);
        }
    #else // POSIX
        DIR* dir = opendir(models_dir_path);
        if (dir) {
            struct dirent* entry;
            while ((entry = readdir(dir)) != NULL) {
                if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                    char model_path[SAFE_PATH_MAX];
                    if (safe_path_join(model_path, sizeof(model_path), models_dir_path, entry->d_name)) {
                        if (is_valid_model_dir(model_path)) {
                            add_model_to_list(model_path, models_out, &count, &capacity);
                        }
                    }
                }
            }
            closedir(dir);
        }
    #endif

    return count;
}