# AlphaZero-Style Self-Play Chess Engine Implementation Plan

This document outlines the complete, end-to-end pipeline for building a self-learning chess engine based on the provided specification. The project utilizes C++17 for the engine core, Python/PyTorch for neural network training, and ONNX Runtime for C++ inference integration.

## User Review Required

> [!IMPORTANT]
> Please review this plan and confirm if you are ready to proceed with Phase 1. 
> Also, let me know if you have a specific subdirectory in mind within your workspace to initialize this project (e.g., `AlphaZeroChess`), or if I should create it in the root of the current repository.

## Required Dependencies

Here is the complete list of tools and libraries we will use. Since you are on Windows, these are the recommended choices:

### 1. C++ Dependencies (Phases 1, 2, 4)
- **C++ Compiler (C++17):** MSVC (Microsoft Visual C++) is highly recommended on Windows. You can install it by downloading **Visual Studio Build Tools 2022** and selecting the "Desktop development with C++" workload.
- **Build System:** **CMake**. We will use CMake to compile our C++ code and easily link it with ONNX Runtime.
- **ONNX Runtime (C++):** We will configure our CMake setup to automatically download the pre-compiled ONNX Runtime library for Windows, so you won't need to install this manually.

### 2. Python Dependencies (Phase 3)
- **Python:** Python 3.10 or newer.
- **PyTorch (with GPU support):** Since you plan to use a local GPU, you will need to install PyTorch with CUDA support. This typically requires installing the [NVIDIA CUDA Toolkit](https://developer.nvidia.com/cuda-downloads).
- **Python Libraries:** 
  - `numpy` (for data manipulation)
  - `onnx` and `onnxruntime` (for exporting and testing the model)

> [!TIP]
> If you haven't installed these yet, I recommend starting with installing **Visual Studio Build Tools 2022**, **CMake**, and **Python**. We can install the specific Python packages (like PyTorch) via `pip` when we reach Phase 3.

## Proposed Changes

The project will be built in four distinct phases:

---

### Phase 1: C++ Chess Engine Core

The foundation of the engine, handling chess logic, move generation, and basic search.

#### [NEW] `src/core/bitboard.h` & `src/core/bitboard.cpp`
- Implement 64-square board representation using 12 bitboards.
- Add key operations (`popcount`, `lsb`) and utilities for square indexing and bit manipulation.

#### [NEW] `src/core/movegen.h` & `src/core/movegen.cpp`
- Implement pseudo-legal and legal move generation for all piece types (Pawns, Knights, Kings, Sliding pieces).
- Implement magic bitboards or lookup tables for sliding piece attacks.

#### [NEW] `src/core/search.h` & `src/core/search.cpp`
- Implement Alpha-Beta pruning search with move ordering and transposition tables.
- Implement a basic static evaluation function (material count, piece-square tables, king safety) to be used before the neural network is ready.

#### [NEW] `src/core/board.h` & `src/core/board.cpp`
- Manage board state, including move application (`apply_move`, `undo_move`), check detection, and game end conditions (checkmate, stalemate, threefold repetition).

#### [NEW] `src/engine/chess_engine.h` & `src/engine/chess_engine.cpp`
- Provide the main public API for the engine (set FEN, get best move, play move, get game result).

---

### Phase 2: Self-Play Data Generation Loop

Generating training data by having the engine play against itself.

#### [NEW] `src/training/self_play.h` & `src/training/self_play.cpp`
- Implement the `play_self_game` algorithm to record positions, moves, and outcomes.
- Configure self-play parameters (search depth, games per iteration).

#### [NEW] `src/training/data_storage.h` & `src/training/data_storage.cpp`
- Implement data serialization to store game records compactly (Binary or JSON format).

#### [NEW] `src/main_training.cpp`
- Create the main training loop executable to run iterations of self-play and save batches of games.

---

### Phase 3: Python Neural Network Training

Training the AlphaZero-style Convolutional Neural Network (ResNet).

#### [NEW] `python/dataset.py`
- Create a PyTorch `Dataset` class (`ChessGameDataset`) to load binary/JSON game records.
- Implement the `board_to_tensor` function to convert the C++ board state into a 119-plane tensor.

#### [NEW] `python/model.py`
- Define the `ChessNet` architecture using PyTorch, including residual blocks, a policy head (for move probabilities), and a value head (for position evaluation).

#### [NEW] `python/train.py`
- Implement the training loop (`ChessTrainer`) with loss computation (cross-entropy for policy, MSE for value), L2 regularization, and learning rate scheduling.

#### [NEW] `python/export_onnx.py`
- Implement a script to export the trained PyTorch model to the ONNX format (`chess_model.onnx`) for integration with C++.

---

### Phase 4: C++ ONNX Integration & MCTS

Integrating the trained neural network back into the C++ engine using Monte Carlo Tree Search.

#### [NEW] `src/mcts/onnx_evaluator.h` & `src/mcts/onnx_evaluator.cpp`
- Create a wrapper around ONNX Runtime C++ API to load the `.onnx` model and perform batched inference, returning policy and value estimates.

#### [NEW] `src/mcts/mcts.h` & `src/mcts/mcts.cpp`
- Implement the Monte Carlo Tree Search (MCTS) algorithm with PUCT (Predictor + Upper Confidence Bound applied to Trees).
- Implement node expansion, leaf evaluation using the ONNX network, and backpropagation.

#### [MODIFY] `src/engine/chess_engine.h` & `src/engine/chess_engine.cpp`
- Replace or supplement the basic Alpha-Beta search with the MCTS engine.

## Verification Plan

### Automated Tests
- **Phase 1**: Unit tests for bitboard operations, move generation (perft testing to verify move counts at various depths), and game termination logic.
- **Phase 2**: Verify output data format and ensure generated self-play games contain only legal moves and end correctly.
- **Phase 3**: Verify tensor conversions, monitor training loss convergence, and validate ONNX export output shapes.
- **Phase 4**: Run engine vs. engine matches (MCTS vs Alpha-Beta) to ensure MCTS evaluates and plays correctly without crashing.

### Manual Verification
- Reviewing sample FENs and ensuring the engine selects reasonable moves.
- Checking resource usage (memory/CPU/GPU) during self-play and training to ensure there are no severe bottlenecks or leaks.
