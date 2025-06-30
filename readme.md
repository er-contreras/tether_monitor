# Tether Monitor

A lightweight terminal-based tool written in C that tracks real-time internet data usage (RX and TX bytes) on Linux systems — ideal for USB tethering or general interface monitoring.

## 🚀 Features

- 📡 Monitors data received (RX) and transmitted (TX) via a network interface.
- 🔄 Tracks **delta**: the difference in RX/TX usage between each update.
- 🕒 Displays live updates in the terminal.
- 🧪 Designed to be simple, fast, and hackable.

## 📸 Example Output

```bash
$ ./tether_monitor
RX: 76.90 MB (+0.28 MB) | TX: 4.96 MB (+0.01 MB)
RX: 77.12 MB (+0.22 MB) | TX: 4.98 MB (+0.02 MB)
RX: 77.36 MB (+0.24 MB) | TX: 5.01 MB (+0.03 MB)

