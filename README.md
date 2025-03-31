# C Chess

![CMake](https://img.shields.io/badge/CMake-%23008FBA.svg?style=for-the-badge&logo=cmake&logoColor=white)

A fully-featured chess game implemented in C using a modular architecture. Play classic chess in your terminal with all standard rules and moves supported.

## Features

- Complete implementation of chess rules
- Terminal-based graphical interface
- Support for standard chess notation
- Move validation and legal move highlighting
- Check and checkmate detection
- Support for special moves (castling, en passant, pawn promotion)
- Game state saving and loading
- Move history and undo functionality
- Simple AI opponent (optional)

## Requirements

- C compiler (GCC or Clang recommended)
- CMake (version 3.12 or higher)
- Make
- Standard C libraries

## Building from Source

Clone the repository:

```bash
git clone https://github.com/mc-ondros/c_chess.git
cd c_chess
```

Build using CMake:

```bash
mkdir build
cd build
cmake ..
make
```

## Running the Game

From the build directory:

```bash
./c_chess
```

## Technical Implementation

This chess game is built with the following design considerations:

- **Board Representation**: 8x8 grid using a 2D array or bitboards
- **Move Generation**: Pre-calculated move tables for efficiency
- **Game State**: Full position tracking with history for undo
- **User Interface**: ASCII/Unicode character-based terminal UI
- **AI**: Mini-max algorithm with alpha-beta pruning (basic implementation)

## Future Improvements

- Graphical interface using SDL or similar library
- Network play functionality
- Advanced AI with opening book
- PGN format support for importing/exporting games
- Time controls and chess clock
- Tournament mode

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add some amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Acknowledgments

- Thanks to the classic chess literature and algorithms
- Inspired by traditional command-line chess implementations
- Chess piece Unicode characters for the terminal display
