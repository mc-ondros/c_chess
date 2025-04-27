# C Chess Game

![C](https://img.shields.io/badge/language-C-blue)
![CMake](https://img.shields.io/badge/build-CMake-brightgreen)
![License](https://img.shields.io/github/license/mc-ondros/c_chess)
![Repo Size](https://img.shields.io/github/repo-size/mc-ondros/c_chess)
![Last Commit](https://img.shields.io/github/last-commit/mc-ondros/c_chess)

## Description

Welcome to the C Chess Game! This project is an engaging and educational chess game implemented entirely in C, featuring a graphical user interface built with GTK. It serves as an excellent example for learning game development, GUI programming with GTK, data structures, and algorithm implementation in C.

Play against a friend in **two-player mode** or challenge a robust AI opponent powered by the Gemini engine, offering a challenging experience for players of all levels. Whether you're a seasoned chess player or a beginner, this game provides an interactive way to enjoy chess.

When playing against the AI, you can customize its personality to adjust its playing style and difficulty, making each game unique. Additionally, you can request the AI to rate your moves, providing valuable feedback to help improve your game.

## Features

*   **Standard Chess Rules:** Implements all traditional chess rules including piece movements, capturing, castling, en passant, and pawn promotion.
*   **Graphical User Interface (GUI):** Modern and intuitive interface built with GTK for easy interaction and gameplay.
*   **Two-Player Mode:** Play against another human opponent on the same computer.
*   **Challenging AI Opponent:** Play against an AI opponent powered by the sophisticated Gemini engine.
*   **AI Personality Customization:** Tailor the AI's playing style and difficulty level when playing against the computer.
*   **AI Move Rating:** Option to request the AI to evaluate your moves and provide constructive feedback (AI mode only).
*   **Move Validation:** Ensures all attempted moves are legal according to the rules of chess before they are executed.
*   **Save and Load Game:** Allows you to save your current game progress and load it later to continue your match.
*   **Clear Board Display:** Visual representation of the chessboard and piece positions within the GUI.
*   **Check/Checkmate/Stalemate Detection:** Automatically detects game-ending conditions like checkmate and stalemate.

## Table of Contents

- [Usage](#usage)
- [License](#license)
- [Acknowledgements](#acknowledgements)

## Usage

After building the project, launch the executable (e.g., `./c_chess` from the build directory or via your system's application launcher if installed).

Use your mouse to interact with the graphical interface:
-   **Select Game Mode:** Choose between playing against the AI or a local human opponent.
-   **Move Pieces:** Click on the piece you want to move, and then click on the destination square. Valid moves are typically highlighted.
-   **AI Settings:** If playing against the AI, configure its personality/difficulty via the game's menus or settings options.
-   **Get Move Rating:** Use the corresponding menu option or button during your turn (if available in AI mode).
-   **Save/Load Game:** Access the save and load functions through the game's menu bar (e.g., File -> Save).

## License

This project is licensed under the terms specified in the LICENSE file.

## Acknowledgements

-   Thanks to the developers of the GTK toolkit for the GUI framework.
-   Thanks to the developers of the Gemini engine for the AI capabilities.
-   Inspired by classic chess games.
