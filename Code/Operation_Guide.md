# RPS-1 Code Run Guide

This document describes how we run and operate the Arduino code for our RPS-1 system.

## 1. Prerequisites

- Arduino IDE installed
- Arduino Uno R3 connected
- Servos connected to the pins used in code:
  - Left MI: pin 5
  - Left PRT: pin 6
  - Right MI: pin 10
  - Right PRT: pin 11
- Servo power available and stable

## 2. Arduino IDE Setup

1. Open Arduino IDE.
2. Open the required sketch file.
3. Set board to Arduino Uno.
4. Select the correct COM port.
5. Upload the sketch.
6. Open Serial Monitor at 9600 baud.
7. Set line ending to Newline.

## 3. Sketch Purpose

- `HandTuner/HandTuner.ino`
  - Used to tune per-servo STOP, LOCK, speed, and duration values.
  - Use this when calibration is needed.

- `HandActions/HandActions.ino`
  - Used for manual gesture checks and quick hardware validation.
  - Use this to verify each hand can perform Rock, Paper, Scissors, and HOME.

- `FullGameComplete/FullGameComplete.ino`
  - Used for full game operation in rounds.
  - Use this for live gameplay and tournament operation.

## 4. Tournament Operation

After uploading `FullGameComplete.ino`:

1. Type `HELP` to list commands.
2. Type `HOME` to reset both hands.
3. Type `ROUND` to start a new round.
4. When prompted for opponent Stage 1 input, enter two letters (example: `RP`).
5. When prompted for opponent kept hand and result, enter format `<hand> <result>` (example: `R W`).
6. Repeat with `ROUND` for the next round.
7. Use `STATS` at any time to view summary results.

## 5. Helpful Commands

- `HOME` : return hands to open state
- `STATE` : print current motor/finger state
- `STATS` : print win/loss/tie and strategy statistics
- `LR RR LS RS LP RP LIV RIV` : manual gesture override commands
- `CLR`, `CLR L`, `CLR R`, `OPN`, `OPN L`, `OPN R` : reset/open overrides

