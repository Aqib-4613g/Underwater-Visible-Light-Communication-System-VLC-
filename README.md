# Underwater Visible Light Communication (VLC) System

## Overview

This project implements a **bidirectional Visible Light Communication (VLC) system** using two custom ESP32-based boards. Digital data is transmitted optically using LEDs and received via photodiodes.

All communication protocol logic is implemented in embedded firmware. MATLAB is used only as a serial interface for message input.

### Key Features

* Character-based transmission
* Frame-based encoding
* Preamble synchronization
* Start/Stop bit validation
* End-of-message handling
* Optical ACK handshake
* MATLAB-based message interface

---

## System Architecture

### Transmitter Node

* White LED for optical transmission
* Photodiode for ACK reception
* Serial input (Arduino IDE / MATLAB)

### Receiver Node

* Photodiode for data reception
* Blue LED for ACK transmission
* Serial output for decoded messages

### Communication Flow

```
MATLAB → Transmitter ESP32 → Optical Channel → Receiver ESP32 → Serial Output
Receiver ESP32 → Optical ACK → Transmitter ESP32
```

---

## Communication Protocol

### Bit Timing

* Bit duration: **5 ms**
* Bit rate: **200 bps**
* Effective data rate: **160 bps** (8 data bits / 10-bit frame)
* Mid-bit sampling for stability

---

### Frame Structure

Each character is transmitted as a 10-bit frame:

```
[Start Bit] [8 Data Bits - MSB First] [Stop Bit]
     1            XXXXXXXX                0
```

Example (`'A'`):

```
1 01000001 0
```

---

### Preamble

Pattern:

```
10101010
```

Purpose:

* Receiver synchronization
* Clock alignment
* Start-of-message detection

The receiver begins decoding only after detecting this sequence.

---

## End-of-Message Behavior

### Current Firmware Status

The transmitter no longer sends the NULL termination frame:

```cpp
// sendFrame((char)0x00);
```

However, the receiver still contains logic to terminate reception upon detecting:

```
1000000000
```

### Current Effect

* No explicit end-of-message frame is transmitted.
* Reception ends due to timing and idle optical conditions.
* Termination is **timing-driven**, not frame-driven.

### Recommendation

To maintain protocol symmetry:

**Option A (Recommended):**
Re-enable NULL frame transmission in the transmitter.

**Option B:**
Modify the receiver to use timeout-based termination instead of NULL detection.

The system functions correctly but is not structurally symmetric in its current state.

---

## Receiver Operation

1. Wait for idle optical state
2. Detect preamble (10101010)
3. Decode incoming frames
4. Validate start and stop bits
5. Print decoded message
6. Transmit ACK

### ACK Transmission

* Wait: `bitDelay × 150`
* Send preamble
* Guard delay: `bitDelay × 20`
* Send framed characters: `A`, `C`, `K`
* Send two LOW bits for optical stabilization

---

## MATLAB Integration

MATLAB acts strictly as a serial interface layer.

* COM Port: `COM7`
* Baud Rate: `115200`
* Uses `writeline()` (newline-terminated)
* Serial connection initialized once at startup
* 2.2-second delay for ESP32 auto-reset
* Debug logs read for a fixed duration
* Loop continues until user exits

All encoding and decoding are handled in ESP32 firmware.

---

## Hardware Design

Each custom board includes:

* ESP32-WROOM module
* LED driver stage
* Photodiode input stage
* Gate control pin
* USB-to-UART interface
* 5V and 3.3V regulation

Communication is line-of-sight.
Underwater deployment requires wavelength selection and attenuation considerations.

---

## System Characteristics

### Advantages

* Immune to RF interference
* Secure line-of-sight communication
* Suitable for underwater environments

### Limitations

* Requires optical alignment
* Sensitive to ambient light
* Limited transmission range

---

## Future Improvements

* CRC-based error detection
* Manchester encoding
* Adaptive photodiode thresholding
* Automatic gain control
* Image transmission via MATLAB
* Full-duplex communication

---

## Conclusion

This project demonstrates a fully embedded, frame-based VLC protocol implemented on ESP32 hardware with bidirectional acknowledgment support.

It validates:

* Optical digital transmission
* Reliable synchronization via preamble
* Structured frame decoding
* Bidirectional optical handshake

The system provides a strong foundation for higher data-rate underwater optical communication and future image-based transmission systems.
