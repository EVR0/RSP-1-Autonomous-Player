# RSP-1 Autonomous Player

We are Team MSE-08, and this repository contains our final design and implementation for a Rock-Paper-Scissors Minus One (RPS-1) robotic hand system.

## Project Overview

- Course: MSE 3302B - Sensors and Actuators
- Team: MSE-08

### Team Members

- Evan Romano (251327329)
- Joseph Toma (251283541)
- Mohammed Alamen Qassab (251285296)

### Primary Objectives

- Produce stable Rock, Paper, and Scissors gesture outputs.
- Ensure repeatable transitions between gestures.
- Simplify manufacturing, assembly, and maintenance.

### Engineering Constraints

- Limited actuator count and wiring complexity.
- Need for quick iteration and debugging.
- Mechanical robustness for repeated demonstrations.

## Design and Development Process

Our development process was practical and iterative:

- We started from a hand model reference found online and used it as a baseline for geometry and mechanism ideas.
- We modified that model to house electronic components, improve assembly fit, and support reliable finger actuation for our RPS-1 use case.
- We then iterated clearances, mounting points, and routing until gesture motion was repeatable and demo-ready.

## Technical Specifications and Design Decisions

We used weighted decision matrices to choose the core subsystem concepts and then implemented the final selected designs.

### Actuation System

- Implemented: String-pulley based actuation
- Decision basis: high controllability with lower manufacturing complexity than the alternatives we evaluated

### Finger Retention

- Implemented: Elastic band return mechanism
- Decision basis: reduced motor count and improved manufacturability

### Grouped Actuation Strategy

To reduce wiring complexity, power demand, and actuator count, we split finger actuation into two sets:

- Set 1: Pinky, Ring, and Thumb
- Set 2: Index and Middle

## Electronics Selection

- Actuators: FS90R Servo Motors
  - Chosen for practical integration and straightforward control.
- Controller: Arduino Uno R3
  - Chosen for reliable PWM generation and stable debug workflow.

## Mechanical Reference Used

For hand mechanism development and build guidance, we used this hand-model playlist as a primary practical reference:

- https://youtube.com/playlist?list=PLPE6SxiX3tFlu7DIgrNp9rpRh4uXIouPx&si=-t2cuAqJtvJ5-hMz

## Repository Structure

This repository currently contains these main folders:

- `CAD/`
  - CAD models, assemblies, and print-related mechanical files.

- `Code/`
  - Embedded Arduino code and final control logic.
  - Includes:
    - `Code/HandTuner/HandTuner.ino`: per-servo calibration/tuning utility.
    - `Code/HandActions/HandActions.ino`: manual gesture command and action testing.
    - `Code/FullGameComplete/FullGameComplete.ino`: integrated round flow and final game operation.

- `Media/`
  - Project media assets for documentation and presentation support.

- `Documentation/`
  - Reports and presentation used for analysis and final submissions.
  - Includes:
    - `RPS1_Optimal_Strategy_Analysis.ipynb`
    - `FinalReport.ipynb`
    - `Project Presentation - MSE 3302B - 8`
