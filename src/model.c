#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "model.h"
#include "utils.h"

// Function to load a float array from a CSV file
static int load_float_array_from_csv(const char* filepath, float* array, int num_elements) {
    FILE* fp = fopen(filepath, "r");
    if (fp == NULL) {
        fprintf(stderr, "ERROR: Could not open file %s\n", filepath);
        return 0; // Failure
    }

    for (int i = 0; i < num_elements; i++) {
        // fscanf will read a float and skip over commas and whitespace
        if (fscanf(fp, "%f,", &array[i]) != 1) {
            fprintf(stderr, "ERROR: Failed to read element %d from %s\n", i, filepath);
            fclose(fp);
            return 0; // Failure
        }
    }

    fclose(fp);
    return 1; // Success
}

TinyNN_Model* create_model_from_path(const char* model_path) {
    char filepath[256];

    // 1. Read architecture file
    snprintf(filepath, sizeof(filepath), "%s/architecture.txt", model_path);
    FILE* fp = fopen(filepath, "r");
    if (fp == NULL) {
        fprintf(stderr, "ERROR: Could not open architecture file at %s\n", filepath);
        return NULL;
    }

    TinyNN_Model* model = (TinyNN_Model*)malloc(sizeof(TinyNN_Model));

    // Read sizes from the file
    fscanf(fp, "%d", &model->input_size);
    fscanf(fp, "%d", &model->output_size);
    fscanf(fp, "%d", &model->hidden_layers);

    int total_layers = model->hidden_layers + 1;
    model->layer_sizes = (int*)malloc(sizeof(int) * total_layers);
    for (int i = 0; i < total_layers; i++) {
        fscanf(fp, "%d", &model->layer_sizes[i]);
    }
    fclose(fp);

    // 2. Allocate memory for weights and biases
    model->weights = (float**)malloc(sizeof(float*) * total_layers);
    model->biases  = (float**)malloc(sizeof(float*) * total_layers);

    int prev_layer_size = model->input_size;
    for (int i = 0; i < total_layers; i++) {
        int current_layer_size = model->layer_sizes[i];
        int weight_count = prev_layer_size * current_layer_size;
        int bias_count = current_layer_size;

        model->weights[i] = (float*)malloc(sizeof(float) * weight_count);
        model->biases[i]  = (float*)malloc(sizeof(float) * bias_count);

        // 3. Load weights and biases from their respective CSV files
        snprintf(filepath, sizeof(filepath), "%s/layer_%d_weights.csv", model_path, i);
        if (!load_float_array_from_csv(filepath, model->weights[i], weight_count)) {
            // On failure, free everything allocated so far and return NULL
            free_model(model);
            return NULL;
        }

        snprintf(filepath, sizeof(filepath), "%s/layer_%d_biases.csv", model_path, i);
        if (!load_float_array_from_csv(filepath, model->biases[i], bias_count)) {
            free_model(model);
            return NULL;
        }

        prev_layer_size = current_layer_size;
    }

    return model;
}

void free_model(TinyNN_Model* model) {
    if (model == NULL) return; // Safety check

    if (model->weights) {
        for (int i = 0; i <= model->hidden_layers; i++) {
            if (model->weights[i]) free(model->weights[i]);
        }
        free(model->weights);
    }
    if (model->biases) {
        for (int i = 0; i <= model->hidden_layers; i++) {
            if (model->biases[i]) free(model->biases[i]);
        }
        free(model->biases);
    }
    if (model->layer_sizes) free(model->layer_sizes);
    free(model);
}

float* forward_pass(TinyNN_Model* model, float* input) {
    float* current_input = input;
    float* layer_output = NULL;
    int current_input_size = model->input_size;

    // A flag to know if we've allocated memory for current_input that needs freeing later.
    // The initial input is from the user, so we are not freeing it.
    int input_is_dynamically_allocated = 0;

    // Loop through each layer (hidden layers + output layer)
    for (int i = 0; i <= model->hidden_layers; i++) {
        int layer_output_size = model->layer_sizes[i];
        layer_output = (float*)malloc(sizeof(float) * layer_output_size);

        // Core Dense Layer Calculation: output = W * input + b
        for (int j = 0; j < layer_output_size; j++) {
            float sum = 0.0f;
            // Dot product of weights row and current_input vector
            for (int k = 0; k < current_input_size; k++) {
                // Weights are stored as a flat array (row-major order)
                // W[j][k] is equivalent to weights[j * current_input_size + k]
                sum += model->weights[i][j * current_input_size + k] * current_input[k];
            }
            // Add the bias for this neuron
            sum += model->biases[i][j];
            layer_output[j] = sum;
        }

        // Apply Activation Function
        if (i < model->hidden_layers) {
            // Apply ReLU for all hidden layers
            for (int j = 0; j < layer_output_size; j++) {
                layer_output[j] = relu(layer_output[j]);
            }
        } else {
            // Apply Softmax for the final output layer (common for classification)
            softmax(layer_output, layer_output_size);
        }

        // Prepare for the next layer
        if (input_is_dynamically_allocated) {
            free(current_input); // Free the output from the previous layer
        }

        current_input = layer_output; // The output of this layer is the input to the next
        current_input_size = layer_output_size;
        input_is_dynamically_allocated = 1;
    }

    // The final 'current_input' is the network's output.
    // The CALLER is now responsible for freeing this memory.
    return current_input;
}
