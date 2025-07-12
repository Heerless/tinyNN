#ifndef MODEL_MANAGER_H
#define MODEL_MANAGER_H

// *** FIX: Define a single, safe, cross-platform maximum path length. ***
// 4096 is a common value for PATH_MAX on modern Linux systems and is
// much larger than Windows' default, making it safe for both.
#define SAFE_PATH_MAX 4096

// A struct to hold information about a discovered model folder.
typedef struct {
    // *** FIX: Use the new safe max path length. ***
    char path[SAFE_PATH_MAX];
} DiscoveredModel;

/**
 * @brief Runs the interactive process to import an external model.
 * Prompts the user for a source path and a new name, then validates and
 * copies the model into the 'custom_models' directory.
 */
void run_model_importer();

/**
 * @brief Scans the current directory for valid model folders.
 * A folder is considered a valid model if it contains an 'architecture.txt' file.
 *
 * @param models_out A pointer to an array of DiscoveredModel structs that will be
 *                   allocated by this function. The caller is responsible for
 *                   freeing this memory.
 * @return The number of models discovered. Returns 0 on failure or if no models are found.
 */
int discover_models(DiscoveredModel** models_out);

#endif