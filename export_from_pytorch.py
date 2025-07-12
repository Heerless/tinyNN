import torch
import torch.nn as nn
import numpy as np
import os

# 1. Define a PyTorch Model
class MicroModel(nn.Module):
    def __init__(self):
        super(MicroModel, self).__init__()
        self.layer1 = nn.Linear(16, 8)
        self.relu = nn.ReLU()
        self.layer2 = nn.Linear(8, 4)

    def forward(self, x):
        x = self.relu(self.layer1(x))
        x = self.layer2(x)
        return x

# 2. Create and (Optionally) Train the Model
model = MicroModel()
model.eval() # Set to evaluation mode

print("PyTorch model created:")
print(model)

# 3. Export to TinyNN Format
MODEL_DIR = "pytorch_model"
if not os.path.exists(MODEL_DIR):
    os.makedirs(MODEL_DIR)

print(f"\nExporting model to directory: {MODEL_DIR}")

# A. Save Architecture File
input_size = model.layer1.in_features
output_size = model.layer2.out_features
layer_sizes = [model.layer1.out_features, model.layer2.out_features]
hidden_layers = len(layer_sizes) - 1

with open(os.path.join(MODEL_DIR, "architecture.txt"), "w") as f:
    f.write(f"{input_size}\n")
    f.write(f"{output_size}\n")
    f.write(f"{hidden_layers}\n")
    for size in layer_sizes:
        f.write(f"{size}\n")
print("Saved architecture.txt")

# B. Save Weights and Biases
layer_idx = 0
for name, param in model.named_parameters():
    if param.requires_grad:
        data = param.detach().numpy()
        
        if "weight" in name:
            filepath = os.path.join(MODEL_DIR, f"layer_{layer_idx}_weights.csv")
            np.savetxt(filepath, data, delimiter=",")
            print(f"   ✓ Saved {os.path.basename(filepath)}")
        elif "bias" in name:
            filepath = os.path.join(MODEL_DIR, f"layer_{layer_idx}_biases.csv")
            np.savetxt(filepath, data, delimiter=",")
            print(f"   ✓ Saved {os.path.basename(filepath)}")
            layer_idx += 1

print("\nModel export from PyTorch complete!")
print(f"You can now run this model in the TinyNN engine by providing the path '{MODEL_DIR}'.")