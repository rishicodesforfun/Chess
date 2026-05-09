# AlphaZero Chess Engine

A high-performance chess engine implementation based on the AlphaZero architecture, combining a C++ search engine with a PyTorch-trained deep residual neural network.

## 🌟 Overview

AlphaZero Chess is a self-learning chess engine. It uses Monte Carlo Tree Search (MCTS) guided by a deep neural network to evaluate positions and select moves. Unlike traditional engines that rely on hand-crafted heuristics, this engine learns exclusively through self-play data.

### Key Features
- **C++17 Engine Core**: High-speed bitboard-based move generation and state management.
- **Deep Residual Network**: A PyTorch ResNet model with policy and value heads.
- **ONNX Integration**: Seamless inference in C++ using ONNX Runtime.
- **AlphaZero Encoding**: Standard 4672-move policy mapping and 119-plane history input.
- **Zobrist Hashing**: Incremental hashing for position tracking and repetition detection.

## 🚀 Setup Instructions

### Prerequisites
Please refer to [prerequisites.txt](prerequisites.txt) for detailed system requirements and installation steps.

### Building the Engine
1. Open PowerShell and navigate to the project root.
2. Create a build directory and compile:
   ```powershell
   mkdir build
   cd build
   cmake ..
   cmake --build . --config Release
   ```

### Training the Model
1. Activate your Python virtual environment.
2. Run the training script:
   ```powershell
   python python/train.py
   ```

## 🎮 Usage Examples

### Running Self-Play
To generate new training data by having the engine play against itself:
```powershell
.\build\src\Release\AlphaZeroChess.exe
```

### Playing Against the Engine
(Future feature: UCI support) Currently, the engine is optimized for self-play data generation. You can use the `ChessEngine` class in your own C++ projects.

## 🛠️ API Documentation

### C++ Engine (`ChessEngine` Class)
- `set_position(const std::string& fen)`: Sets the board to a specific FEN.
- `get_best_move(int depth_or_sims)`: Returns the best move found via MCTS or Alpha-Beta.
- `play_move(Move m)`: Executes a move on the board.
- `is_game_over()`: Checks if the current game has concluded.

### Python Data Pipeline
- `ChessGameDataset`: A PyTorch dataset for loading `.jsonl` game records.
- `ChessNet`: The residual neural network architecture.

## 🤝 Contribution Guidelines

Contributions are welcome! Please follow these steps:
1. Fork the repository.
2. Create a feature branch (`git checkout -b feature/AmazingFeature`).
3. Commit your changes (`git commit -m 'Add AmazingFeature'`).
4. Push to the branch (`git push origin feature/AmazingFeature`).
5. Open a Pull Request.

## 📜 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details (to be added).

---
*Built with ❤️ by the AlphaZero Chess Team.*
