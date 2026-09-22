# Espie

Espie is a local n-gram language model designed to run directly on an ESP32-S3.

It does not use an API, cloud AI service, or external LLM runtime. The model, corpus, inference system, sampling system, and command interface run locally on the microcontroller.

Espie can have conversations, complete sentences, generate longer pieces of text, use topic-specific corpus sections, retrieve verified facts, learn additional prose, and expose its internal behavior through debugging and control commands.

## Features

- Local language generation on ESP32-S3
- N-gram language model
- Corpus-based text generation
- Conversation detection
- Sentence completion
- Topic-based generation
- Factual knowledge retrieval
- `/story` long-form generation
- `/raw` unrestricted literal completion
- `/gen` controlled token generation
- `/topic` corpus section selection
- `/learn` persistent user-provided learning
- `/forget` learned-data removal
- `/class` text classification
- `/calc` arithmetic expression evaluation
- Temperature control
- N-gram order control
- Generation-length control
- Reproducible RNG seeds
- Inference debug tracing
- Runtime statistics
- Generation benchmarking
- No internet connection required for inference

## Hardware

Espie is designed for the **ESP32-S3**.

The exact amount of available RAM and flash depends on the ESP32-S3 board being used.

The project is particularly suited for boards with enough flash and RAM to store the corpus and run the inference system comfortably.

## Requirements

### Hardware

- ESP32-S3 development board
- USB connection
- Computer for uploading the firmware

### Software

- Arduino IDE
- ESP32 Arduino board support
- ESP32-S3 board configuration

## Installation

1. Install the Arduino IDE.

2. Install ESP32 board support through the Arduino Board Manager.

3. Open the Espie project.

4. Make sure the project contains:

```text
Espie/
├── Espie.ino
└── training_data.h
