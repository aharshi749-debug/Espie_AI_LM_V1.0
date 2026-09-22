# Espie

Espie is a local n-gram language model designed to run directly on an ESP32-S3.

It generates text locally without APIs, cloud services, or external LLM runtimes.

## Features

- Local n-gram language model
- Conversation and sentence completion
- Topic-based generation
- Factual knowledge system
- Learning system
- Long-form generation
- Configurable generation
- Debugging and benchmarking
- Offline operation

## Requirements

- ESP32-S3
- Arduino IDE
- ESP32 Arduino board support

## Installation

1. Open `Espie.ino` in Arduino IDE.
2. Select your ESP32-S3 board.
3. Upload the firmware. (WARNING, MUST DO FIRST: HAVE THE FILES NAMED Espie.ino AND training_data.h OR IT WONT COMPILE AT ALL.)
4. Open Serial Monitor at `115200` baud.

Then type normally:

Hi

Espie can also complete text:## Commands

Espie includes a built-in command system.

Run:

/help

to see all available commands and their usage.

## Modifying Espie

Main files:

Espie.ino
training_data.h

Modify `Espie.ino` to change the firmware and model behavior.

Modify `training_data.h` to change or expand the corpus.

Corpus quality has a major effect on generated text.

## Credits

Created and developed by Aharshi.

GitHub: @aharshi749-debug
Wokwi: https://wokwi.com/projects/470533331374442497

Notice: Wokwi link is always guaranteed to be updated more frequently than the github repo.
## License

MIT License

Copyright (c) 2026 Aharshi

See `LICENSE` for the full license.

## Disclaimer

Espie is a probabilistic n-gram language model. Generated text is not guaranteed to be accurate.

Espie v1.0 - Local AI on ESP32-S3.
