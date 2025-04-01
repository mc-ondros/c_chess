# C Chess Game

![C](https://img.shields.io/badge/language-C-blue)
![CMake](https://img.shields.io/badge/build-CMake-brightgreen)
![License](https://img.shields.io/github/license/mc-ondros/c_chess)
![Repo Size](https://img.shields.io/github/repo-size/mc-ondros/c_chess)
![Last Commit](https://img.shields.io/github/last-commit/mc-ondros/c_chess)

## Description

Welcome to the C Chess Game! This project is an engaging and educational chess game implemented entirely in C. It serves as an excellent example for learning game development, data structures, and algorithms in C. The project is built using CMake for better dependency management and build configuration.

The game features a robust AI powered by the Gemini engine, offering a challenging experience for players of all levels. Whether you're a seasoned chess player or a beginner, this game provides an enjoyable and interactive way to sharpen your skills.

You can customize the AI's personality to adjust its playing style and difficulty, making each game unique. Additionally, you can request the AI to rate your moves, providing valuable feedback to help you improve your chess strategies.

## Features

- **Standard Chess Rules:** Play with all the traditional chess rules including castling, en passant, and pawn promotion.
- **Command-line Interface:** Simple and intuitive CLI for easy interaction and control.
- **Save and Load Game:** Save your game progress and load it later to continue your match.
- **AI Opponent:** Challenge yourself against an AI opponent powered by the Gemini engine .
- **Move Validation:** Ensures all moves are legal according to chess rules.
- **AI Personality Customization:** Adjust the AI's playing style and difficulty.
- **Move Rating:** Request the AI to rate your moves and provide feedback.

## Table of Contents

- [Installation](#installation)
- [Usage](#usage)
- [Contributing](#contributing)
- [License](#license)
- [Acknowledgements](#acknowledgements)

## Installation

### Prerequisites

- C Compiler (GCC or Clang)
- CMake

### Steps

1. Clone the repository:
    ```sh
    git clone https://github.com/mc-ondros/c_chess.git
    cd c_chess
    ```

2. Create a build directory and navigate to it:
    ```sh
    mkdir build
    cd build
    ```

3. Generate the build files using CMake:
    ```sh
    cmake ..
    ```

4. Build the project:
    ```sh
    make
    ```

## Usage

After building the project, you can run the chess game using the following command from the `build` directory:

```sh
./c_chess
```

### Command-line Interface

- **Specify AI Personality:** Enter the type of player you want the AI to be (e.g. 'beginner').
- **Move a piece:** Enter the move in the format `e2e4` (move piece from e2 to e4).
- **Let the AI rate your move:** Select y/n if you want the AI to rate your move.

