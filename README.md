# Espie AI LM V1.0

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

Espie can also complete text: E.G "ESP32 is" and it will finish it.

## Commands

Espie includes a built-in command system.

Run:

/help

to see all available commands and their usage.


## Images of Espie running

<img width="1602" height="861" alt="image" src="https://github.com/user-attachments/assets/820e312d-0827-419c-bece-5b98de036172" />

<img width="1556" height="869" alt="image" src="https://github.com/user-attachments/assets/e234fdb9-8390-4f50-a56b-f92cf2ad49c2" />

<img width="1427" height="859" alt="image" src="https://github.com/user-attachments/assets/e4a37836-9aaa-45d8-83f8-f47361bbd620" />

<img width="1917" height="871" alt="image" src="https://github.com/user-attachments/assets/a2cd0a37-3d3a-47a0-b4d2-adc6f40b96c2" />

(Full paragraph since it was cut off in the image: An ESP32 can communicate with peripherals using interfaces such as SPI and I2C. Embedded programming often requires careful management of memory and timing. The ESP32 is a family of microcontrollers designed for embedded applications.
The ESP32-S3 is a member of the ESP32 family. GPIO pins can be configured for digital input and output. UART is a serial communication interface.
A bootloader can load or start firmware. Embedded systems can be found in appliances, vehicles, sensors, controllers, and many other devices. An ESP32 can run networking, control, sensing, and other applications.
An ESP32-S3 can run local software without requiring a cloud service. Running an AI system on a microcontroller requires careful resource management. Flash can store model data while RAM holds active working data.
The ESP32-S3 is a member of the ESP32 family. GPIO pins can be configured for digital input and output. UART is a serial communication interface.)

<img width="1911" height="859" alt="image" src="https://github.com/user-attachments/assets/ddd1ad3f-cb59-457c-97c6-6225e395784a" />

## Modifying Espie

Main files:

Espie.ino
training_data.h

Modify `Espie.ino` to change the firmware and model behavior.

Modify `training_data.h` to change or expand the corpus.

Corpus quality has a major effect on generated text.

## Credits

Created and developed by Aharshi.
Images of the website: 
GitHub: @aharshi749-debug
Wokwi: https://wokwi.com/projects/470533331374442497

Notice: Wokwi link is always guaranteed to be updated more frequently than the github repo.
## License

MIT License

Copyright (c) 2026 Aharshi

See `LICENSE` for the full license.

## Disclaimer

Espie is a probabilistic n-gram language model. Generated text is not guaranteed to be accurate for obvious reasons

Espie v1.0 - Local AI on ESP32-S3.

## Final words

If there are any bugs or issues, please report them to me at aharshi749@gmail.com
