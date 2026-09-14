Folder storing the smart-camper van nodes. The nodes will be running on 8 bit AVRs with FreeRTOS and communicate over SPI to a 10Base-T1S PHY back to the hub.

They will have EEPROM node IDs on each device so that they can run the same binary and do discovery on boot to determine their node.

They are responsible for the following in individual nodes:
1. Lighting switches (GPIO??)
2. Lighting controls (PWM/GPIO/???)
3. Tank water level readings (CAP touch)
4. HVAC controls (GPIO/PWM?)
5. Gray water alarm signalling (CAP touch/GPIO via external device)