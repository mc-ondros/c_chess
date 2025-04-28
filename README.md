# C Chess Game

![C](https://img.shields.io/badge/language-C-blue)
![CMake](https://img.shields.io/badge/build-CMake-brightgreen)
![License](https://img.shields.io/github/license/mc-ondros/c_chess)
![Repo Size](https://img.shields.io/github/repo-size/mc-ondros/c_chess)
![Last Commit](https://img.shields.io/github/last-commit/mc-ondros/c_chess)

## Description

Welcome to the C Chess Game! This project is an engaging and educational chess game implemented entirely in C, featuring a graphical user interface built with GTK. It serves as an excellent example of combining classic board game logic, GUI programming, and AI.

Play against a friend in **two-player mode**, challenge the robust **Gemini AI engine** (cloud-based), or try the new built-in **Local CPU AI** (offline, minimax-based). Whether you're a seasoned chess enthusiast or learning the game, there’s a mode for you.

When playing against the AI, you can customize its personality to adjust its playing style and difficulty, making each game unique. Additionally, you can request the AI to rate your moves, providing feedback and helping you improve your chess skills.

---

## Features

* **Standard Chess Rules:** Implements all traditional chess rules including piece movements, capturing, castling, en passant, pawn promotion, the 50-move rule, and threefold repetition for draw detection.
* **Graphical User Interface (GUI):** Modern and intuitive interface built with GTK for easy interaction and gameplay.
* **Multiple Game Modes:**
  * **Two-Player Mode:** Play against another human opponent on the same computer.
  * **AI Opponent (Gemini):** Play against a sophisticated AI powered by the Gemini engine (requires internet & API key).
  * **Local CPU AI:** Play against a built-in minimax-based AI engine (no internet required).
* **AI Personality Customization:** Tailor the Gemini AI's playing style and difficulty level when playing against the computer.
* **AI Move Rating:** Option to request the AI to evaluate your moves and provide constructive feedback (AI mode only).
* **Move Validation:** Ensures all attempted moves are legal according to the rules of chess before they are executed.
* **Save and Load Game:** Allows you to save your current game progress and load it later to continue your match.
* **Clear Board Display:** Visual representation of the chessboard and piece positions within the GUI.
* **Check/Checkmate/Stalemate/Draw Detection:** Automatically detects game-ending conditions like checkmate, stalemate, 50-move rule, and threefold repetition.
* **Performance Optimizations:** Uses bitboard representation and hashing for fast move generation and efficient draw detection.

---

## Table of Contents

- [Features](#features)
- [Requirements](#requirements)
- [Installation and Build](#installation-and-build)
- [Usage](#usage)
- [Troubleshooting](#troubleshooting)
- [License](#license)
- [Acknowledgements](#acknowledgements)

---

## Requirements

- **Operating System:** Linux, Windows, or macOS.
- **Dependencies:**
  - [GTK+ 3.0](https://www.gtk.org/) (GUI)
  - [libcurl](https://curl.se/libcurl/) (optional, for Gemini AI API)
  - C Compiler (GCC/Clang/MSVC)
  - [CMake](https://cmake.org/) >= 3.10

---

## Installation and Build

### 1. Clone the repository

```sh
git clone https://github.com/mc-ondros/c_chess.git
cd c_chess
```

### 2. Install dependencies

- **Linux (Debian/Ubuntu):**
  ```sh
  sudo apt-get install build-essential cmake libgtk-3-dev libcurl4-openssl-dev
  ```
- **macOS (Homebrew):**
  ```sh
  brew install cmake gtk+3 curl
  ```
- **Windows:**  
  Use [vcpkg](https://github.com/microsoft/vcpkg) or install dependencies manually.  
  Example:
  ```sh
  vcpkg install gtk:x64-windows curl:x64-windows
  ```

### 3. Build

```sh
mkdir build
cd build
cmake ..
cmake --build .
```
- On Windows, you may need to specify the CURL directory:
  ```sh
  cmake -DCURL_DIR=C:/path/to/curl ..
  ```

### 4. Run

```sh
./c_chess
```
Or launch via your system's application launcher if installed.

---

## Usage

After building the project, launch the executable (e.g., `./c_chess` from the build directory or via your system's application launcher if installed).

### Game Modes

When you start the game, you will be prompted to select a mode:
- **1. One-Player (vs Gemini AI):** Uses the Gemini engine via API (internet required, API key required).
- **2. Two-Player:** Two local human players alternate moves.
- **3. One-Player (vs Local CPU):** Uses the built-in minimax AI (fully offline).

### Gameplay

- Use your mouse to interact with the graphical interface:
  - **Select Game Mode:** Choose between playing against the Gemini AI, local minimax AI, or a local human opponent.
  - **Move Pieces:** Click on the piece you want to move, then click on the destination square. Valid moves are typically highlighted.
  - **AI Settings:** If playing against the Gemini AI, configure its personality/difficulty via the game's menus or settings options.
  - **Get Move Rating:** Use the corresponding menu option or button during your turn (if available in AI mode).
  - **Save/Load Game:** Access the save and load functions through the game's menu bar (e.g., File -> Save).

---

## Troubleshooting

- **Missing dependencies:** Ensure GTK and libcurl (optional) are installed.
- **Build errors:** Double-check your CMake version and dependency paths.
- **DLL issues (Windows):** Ensure required DLLs are in the executable directory.
- **API errors:** For Gemini AI, check that your API key is set correctly.

---

## License

This project is licensed under the terms specified in the LICENSE file.

---

## Acknowledgements

- Thanks to the developers of the GTK toolkit for the GUI framework.
- Thanks to the developers of the Gemini engine for the AI capabilities.
- Inspired by classic chess games.

---

## Recent Updates

### April 2025

- **Local CPU AI Mode:** You can now play against a built-in minimax-based AI engine (no internet required). This is available as a new option in the game mode menu.
- **Performance Optimizations:** The engine now uses bitboard representation and hashing for faster move generation and efficient threefold repetition/50-move rule detection.
- **Enhanced Draw Detection:** Automatic handling of the 50-move rule and threefold repetition.
- **Improved GUI and Usability:** Enhanced interaction and notifications for check, checkmate, stalemate, and draw conditions.
- **Flexible Dependency Handling:** If libcurl is not available, Gemini AI mode is disabled, but local CPU AI mode remains available.

For more details, see the source code and commit history!
