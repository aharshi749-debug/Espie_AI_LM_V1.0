# Espie v2.0 (c) Aharshi Deb 2026

## Brief Introduction

Espie is a local RNN chatbot designed to run directly on an ESP32-S3. It generates responses locally without an API or cloud AI.

## Quick Setup

1. Open `Espie.ino` in Arduino IDE.
2. Select your ESP32-S3 board.
3. Upload the project.
4. Open Serial Monitor at 115200 baud.
5. Talk to Espie.

## What Is Inside the Code?

The code contains:

- RNN inference
- Tokenization
- Text generation
- Conversation state
- Fact lookup
- Name recognition
- Calculator
- Topic detection
- Commands
- Configuration
- Debugging and statistics

## Credits

Created by Aharshi.

## How Does an RNN Work?

An RNN processes information one step at a time while keeping a hidden state. The hidden state carries information from previous inputs, allowing the network to use context.

Espie's RNN takes an input word, processes it through the hidden state, and predicts what word should come next. It repeats this process to generate a response.

## User Agreement
If you are going to compile, you agree to these rules
-Do not modify and redistribute without my permission (Gmail me if you need permission or something else)
-
## Thanks for Reading!

Oh god, you read the full readme. Nobody ever does such a thing!

If there are any problems or major bugs, go ahead and contact me at aharshi749@gmail.com, I will be pleased to hear from you
