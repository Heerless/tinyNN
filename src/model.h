#ifndef MODEL_H
#define MODEL_H

typedef struct {
    int input_size;
    int output_size;
    int hidden_layers;
    int* layer_sizes;     // e.g., [64, 32, 10]
    float** weights;      // All weights flattened by layer
    float** biases;       // All biases per layer
} TinyNN_Model;

TinyNN_Model* create_model_from_path(const char* model_path);
void free_model(TinyNN_Model* model);
float* forward_pass(TinyNN_Model* model, float* input);

#endif