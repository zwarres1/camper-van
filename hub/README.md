folder storing the central hub.

The hardware is a PIC32CX connected to a Riverdi 7 inch display.

The software layer is a Zephyr RTOS managing connections to the nodes via 10BASE-T1S and CANBus. All custom software will be 10Base-T1S but the Victron electronics will be CANBus

Functionally the hub will be responsible for the following:
1. Reading lighting switches (10Base-T1S)
2. Commanding lighting outputs (10Base-T1S)
3. Reading water levels (10Base-T1S)
4. Reading gray water fill (10Base-T1S)
5. Signalling audio output if gray water is filled? (TBD/GPIO?)
6. Reading power information (CAN)
7. HVAC controls (10Base-T1S)
8. Displaying information over RiBUS to a display (SPI)