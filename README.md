# Electronics and Embedded Systems Portfolio

**Author:** Rithvik G  

**Development Environment:** JetBrains (CLion, PyCharm, IntelliJ) | Nobara Linux  

This repository serves as a centralized development environment for bare-metal firmware implementations and Object-Oriented software architectures across various embedded electronics and biomedical engineering projects.

## Repository Architecture and Media Gallery

Physical circuit demonstrations, design revisions, schematics, and engineering notebook entries (including formal latency analyses and mathematical calculations) are organized by project phase within the main directory.

[Link: Browse the Engineering Notebook and Circuit Gallery](./Documentation/)

---

## Project Directory

### Phase 1: Hardware-Debounced Neurological Reflex Tester (NRT-1)
Development of a low-level timing metrics platform to evaluate human response latencies.
* **C++ Architecture:** Bare-metal ATmega328P firmware managing hardware debouncing loops and finite state machine execution.
* **Java Architecture:** Object-Oriented terminal-based software twin for validation.
* **Python Architecture:** OS-level keyboard interrupt handler simulating the physical hardware environment.
