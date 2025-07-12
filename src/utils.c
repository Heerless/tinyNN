#include <math.h>
#include "utils.h"

float relu(float x) {
    return x > 0 ? x : 0;
}

float sigmoid(float x) {
    return 1.0f / (1.0f + expf(-x));
}

void softmax(float* input, int length) {
    float max = input[0];
    for (int i = 1; i < length; i++) {
        if (input[i] > max) max = input[i];
    }

    float sum = 0.0f;
    for (int i = 0; i < length; i++) {
        input[i] = expf(input[i] - max);
        sum += input[i];
    }
    for (int i = 0; i < length; i++) {
        input[i] /= sum;
    }
}
