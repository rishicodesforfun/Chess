# AlphaZero Execution Guide

This document outlines the exact, step-by-step commands you need to run to train your AlphaZero chess engine. 

The AlphaZero algorithm learns through a continuous loop. You can repeat these steps as many times as you want. Every time you complete the loop, your engine gets smarter!

---

## 🔁 The Training Loop

### Step 1: Generate Self-Play Data (C++)
You need the engine to play games against itself to generate data. Open your PowerShell terminal and run the compiled C++ executable:

```powershell
.\build\src\Release\AlphaZeroChess.exe
```
> [!NOTE]
> If you get an error that the file doesn't exist, try `.\build\src\Debug\AlphaZeroChess.exe`.
> **What this does:** The engine uses Monte Carlo Tree Search (MCTS) to play games against itself. It saves every move and outcome into a file called `games_iteration_X.jsonl` in your project folder.

### Step 2: Train the Neural Network (Python)
Now you need to feed the data generated in Step 1 into your PyTorch neural network. Run this command:

```powershell
py python\train.py
```
> **What this does:** The Python script loads all the `.jsonl` files you just generated. It trains the ResNet model to better predict the winning moves. Once training is finished, it automatically exports a new, updated `chess_model.onnx` file.

### Step 3: Repeat!
Go back to Step 1 and run the C++ executable again. 

Because the Python script overwrote the `.onnx` file with the newly trained brain, the C++ engine will automatically load the smarter brain when it starts. The self-play games it generates will now be of much higher quality! 

Take that new data, feed it back into Step 2, and repeat!

---

## 🛠️ Modifying the Engine (Optional)
As you run the loop, you might want to adjust the parameters to make it run longer or train harder. 

- **To generate more games:** Open `src\main_training.cpp` and increase the `for (int i = 0; i < 10; i++)` loop (e.g., to 100 or 1000 games). *If you change this C++ file, you must run the `cmake --build build --config Release` command again to recompile!*
- **To train longer:** Open `python\train.py` and change `epochs=5` to a higher number. You do not need to recompile C++ when changing Python files.
