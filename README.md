# 🧠 TinyNN: A Lightweight C-based Neural Network Inference Engine

![Build](https://img.shields.io/badge/Build-Passing-brightgreen.svg)
![Platform](https://img.shields.io/badge/Platform-Windows%20%7C%20Linux%20%7C%20macOS-blue.svg)
![Dependencies](https://img.shields.io/badge/Dependencies-None!-yellow.svg)
![License](https://img.shields.io/badge/License-MIT-lightgrey.svg)

**TinyNN** is a minimalist, dependency-free, and highly portable deep learning inference engine written entirely in standard C. It is designed from the ground up to be simple, educational, and easy to integrate into any C/C++ project. It can run pre-trained neural network models exported from popular frameworks like PyTorch, bringing the power of deep learning to environments where heavy libraries are not an option.

---

## ✨ Core Features

*   **Zero Dependencies:** Written in pure, standard C. No external libraries are needed, just a C compiler.
*   **Highly Portable:** Compiles and runs on Windows, Linux, and macOS.
*   **Model Importer:** A simple workflow to import models trained and exported from PyTorch.
*   **Interactive Menu:** A user-friendly command-line interface for managing and running models.
*   **Model Generator:** Instantly create dummy models of various sizes for testing and demonstration.
*   **Clear File Format:** Uses a human-readable text and CSV-based format for model architecture and parameters.
*   **Educational:** The project's structure is intentionally clear to serve as a learning tool for understanding how inference engines work under the hood.

## 📖 Table of Contents

1.  [Project Philosophy: The "Why?"](#-project-philosophy-the-why)
2.  [Getting Started: A 5-Minute Quickstart](#-getting-started-a-5-minute-quickstart)
3.  [Core Concepts: How It Works](#-core-concepts-how-it-works)
    *   [The TinyNN Model Format](#the-tinynn-model-format)
    *   [The Forward Pass: From Input to Prediction](#the-forward-pass-from-input-to-prediction)
    *   [Supported Activation Functions](#supported-activation-functions)
4.  [How to Use the Program](#-how-to-use-the-program)
    *   [The Main Menu](#the-main-menu)
    *   [Option 1: Generate a New Model](#-option-1-generate-a-new-model)
    *   [Option 2: Run Inference on a Model](#-option-2-run-inference-on-a-model)
    *   [Option 3: Import an External Model](#-option-3-import-an-external-model)
5.  [The Python Bridge: Exporting from PyTorch](#-the-python-bridge-exporting-from-pytorch)
6.  [Project Directory Structure](#-project-directory-structure)
7.  [How to Compile](#-how-to-compile)
8.  [Future Development & Contributing](#-future-development--contributing)
9.  [License](#-license)

---

## 🚀 Project Philosophy: The "Why?"

In a world dominated by massive deep learning frameworks like TensorFlow and PyTorch, **TinyNN** exists to answer a few simple questions:

*   What does a neural network look like when you strip away all the abstractions?
*   How can you run a trained model in a resource-constrained environment (like a microcontroller or an old computer) without a complex toolchain?
*   How does the math of a forward pass actually translate into a working program?

This project is a demonstration of the fundamentals. It's not designed to be the fastest engine, nor is it for training models. It is a lean, mean, inference machine built for **portability**, **simplicity**, and **learning**.

---

## ⏱️ Getting Started: A 5-Minute Quickstart

Follow these steps to see TinyNN in action with a model you export yourself.

1.  **Compile the C Code:** Use your favorite C compiler to build the project. (See the [Compilation](#-how-to-compile) section for a sample command). This will produce a single executable file.

2.  **Export a Model from Python:**
    *   Ensure you have `Python`, `PyTorch`, and `NumPy` installed.
    *   Run the provided Python script:
        ```bash
        python export_from_pytorch.py
        ```
    *   This will create a new directory named `pytorch_model` containing all the necessary `.txt` and `.csv` files that TinyNN understands.

3.  **Run the TinyNN Executable:**
    *   Execute the compiled program from your terminal. You will be greeted by the main menu.

4.  **Import Your Model:**
    *   Choose option `3. Import External Model`.
    *   When prompted for the path, enter `pytorch_model`.
    *   Give it a memorable name, for example, `my_first_model`.
    *   The program will validate and copy the model into its managed `models/` directory.

5.  **Run Inference!**
    *   Now, choose option `2. Run Inference on a Model`.
    *   Your newly imported model (`models/my_first_model`) will appear in the list. Select it.
    *   Choose an input data source (dummy data is fine for a first run).
    *   Watch as the engine performs the forward pass and prints the prediction probabilities!

You have just successfully taken a model from a high-level Python framework and executed it in a lightweight, native C environment.

---

## ⚙️ Core Concepts: How It Works

Understanding these two concepts is key to using and extending TinyNN.

### The TinyNN Model Format

A "model" in TinyNN is not a single file, but a **directory** containing a specific set of files. This structure makes it both human-readable and easy to generate.

A valid model directory (e.g., `models/my_cool_model/`) must contain:

*   **`architecture.txt`**: The blueprint of the network. This is a plain text file that defines the size and shape of the network's layers. Each value is on a new line.
    *   **Line 1:** An integer for the **Input Size** (e.g., number of features in your data).
    *   **Line 2:** An integer for the **Output Size** (e.g., number of classes for classification).
    *   **Line 3:** An integer for the number of **Hidden Layers**.
    *   **Subsequent Lines:** One integer per line, defining the number of neurons in each layer, starting from the first hidden layer and ending with the output layer.

*   **Layer Files (`.csv`)**: For each layer in the network (from `layer_0` to the final layer), there must be two corresponding files:
    *   **`layer_N_weights.csv`**: A CSV file containing the weight matrix for layer `N`. Each row corresponds to a neuron in the current layer, and each column corresponds to a connection from a neuron in the *previous* layer.
    *   **`layer_N_biases.csv`**: A CSV file containing a single row of bias values, one for each neuron in the current layer `N`.

### The Forward Pass: From Input to Prediction

When you run inference, the engine performs a "forward pass," which is the process of feeding input data through the network layers to get a final prediction.

1.  **Model Loading:** The engine first reads the `architecture.txt` file to understand the network's shape. It then allocates memory and loads all the weights and biases from the corresponding `.csv` files.

2.  **Layer-by-Layer Calculation:** The engine processes the network one layer at a time. For each layer:
    a. It performs a matrix multiplication between the layer's weights and the output from the previous layer (or the initial input data for the first layer).
    b. It adds the layer's bias values to the result of the multiplication.
    c. It applies an **activation function** to this result.

3.  **Activation and Output:**
    *   For all hidden layers, the **ReLU** activation function is applied.
    *   For the final output layer, the **Softmax** activation function is applied. This converts the final numbers into a probability distribution, which is ideal for classification tasks.

4.  **Final Result:** The output of the final layer after the Softmax function is the model's prediction, which the program then displays. The engine carefully manages memory, freeing the intermediate results of each layer as it moves to the next.

### Supported Activation Functions

*   **ReLU (Rectified Linear Unit):** A simple but powerful function used in hidden layers. It turns any negative value into zero and leaves positive values unchanged. This helps the network learn complex patterns efficiently.
*   **Softmax:** Used exclusively on the output layer for classification problems. It takes a vector of arbitrary numbers and transforms them into a probability distribution, where all values are between 0 and 1 and their sum is exactly 1.0.

---

## 💻 How to Use the Program

The application is controlled via a simple interactive menu.

### The Main Menu

Upon starting the program, you will see the main menu:

> ```
> ========================
>     TinyNN Main Menu
> ========================
>   1. Generate a New Model
>   2. Run Inference on a Model
>   3. Import External Model
>   0. Exit
> ------------------------
> ```

### ➤ Option 1: Generate a New Model

This option allows you to create a dummy model with randomized weights and biases. It's useful for testing the engine without needing a pre-trained model.

*   You will be presented with a list of presets, from "Micro" to "Huge," each with different input, output, and layer sizes.
*   After choosing a preset, the generator will create a new directory named `models/generated_model`, overwriting any previous contents.
*   It will then create the `architecture.txt` and all the necessary `weights` and `biases` CSV files with random values.

### ➤ Option 2: Run Inference on a Model

This is the core function of the engine.

1.  **Select a Model:** The program will scan the `models/` directory and list all valid model folders it finds. You will be asked to choose one.
2.  **Choose Input Data:** You have two options for providing input data to the model:
    *   **Dummy Data:** A simple option that creates an input vector where every value is `1.0`.
    *   **Load from CSV:** This allows you to use real data. The program will scan the `data/` directory for `.csv` files and let you choose one.
        > **Note:** The input CSV file should contain a single line of comma-separated floating-point numbers. The number of values should match the model's expected input size. If the file has fewer values, the rest will be filled with zeros.
3.  **View Results:** The engine will perform the forward pass and display the results. For a classification model, this will be a list of classes and their corresponding probabilities, color-coded from red (low probability) to green (high probability).

### ➤ Option 3: Import an External Model

This tool bridges the gap between Python and C. It takes a model you've exported into the TinyNN format and copies it into the managed `models/` directory so it can be used for inference.

1.  **Enter Source Path:** Provide the path to the directory containing the exported model files (e.g., `pytorch_model`).
2.  **Enter New Name:** Give your model a unique name (no spaces). This will be the name of the new folder created inside `models/`.
3.  The importer will then validate the source directory (by checking for `architecture.txt`), create the new model folder, and copy all the files over.

---

## 🐍 The Python Bridge: Exporting from PyTorch

The `export_from_pytorch.py` script is your gateway to running real models. Its purpose is to inspect a trained PyTorch model and write its structure and parameters to disk in the exact format that TinyNN expects.

To use it for your own models, you would typically:

1.  **Define or Load Your Model:** Modify the script to instantiate your own trained PyTorch model class.
2.  **Adjust Export Logic:** The script iterates through the model's named parameters. You may need to ensure the layer indices and filenames match the order of your model's layers. The script correctly handles `Linear` layers out of the box.
3.  **Run the Script:** Executing the script will produce the model directory, ready for importing into TinyNN.

---

## 📂 Project Directory Structure

After using the program, your project folder will look something like this:
.
├── tinynn_executable // The compiled C program
├── models/ // Managed directory for all usable models
│ ├── generated_model/ // A model created by the generator
│ │ ├── architecture.txt
│ │ ├── layer_0_weights.csv
│ │ └── ...
│ └── my_first_model/ // A model you imported
│ ├── architecture.txt
│ └── ...
├── data/ // Place your input CSV files here
│ └── sample_input.csv
├── pytorch_model/ // The raw output from the Python export script
│ ├── architecture.txt
│ └── ...
├── export_from_pytorch.py // The Python script for exporting models
└── ... (all the .c and .h source files)

---

## 🔧 How to Compile

TinyNN has no external dependencies, so compiling is straightforward. You will need a C compiler like `gcc` or `clang`. Because the project uses mathematical functions like `expf`, you need to link against the math library.

**On Linux or macOS:**

```bash
gcc main.c model.c model_manager.c generate_model.c utils.c -o tinynn -lm
```

On Windows (with MinGW/GCC):
```bash
gcc -Wall -O2 -o tinynn src/main.c src/model.c src/utils.c src/generate_model.c src/model_manager.c -lm
```
