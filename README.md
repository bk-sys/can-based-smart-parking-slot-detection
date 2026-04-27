Smart Parking System using CAN Protocol (STM32)
This project implements a smart parking system using STM32 and CAN communication. It detects vehicle presence and transmits slot availability from transmitter (TX) to receiver (RX) in real-time.

List your hardware:
STM32 microcontrollerx2
CAN transceiver (MCP2551 or similar)
IR sensorsx2
ultrasonic sensorx2
uart
LCD display
Jumper wires, breadboard
System Architecture
Sensor → STM32 (TX) → CAN Bus → STM32 (RX) → Display
Images
