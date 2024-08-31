# Morse Code Decipherer
A project that deciphers morse code sent from an LED thats connected to a Raspberry Pi 4 through GPIO. The signal is recieved using a photodiode connected to the Analog-Digital Converter (ADC) channel on the ESP32-C3 SoC board. The program is written in C to translate the message.

## Setup
On a breadboard, I connected an led to the Raspberry Pi board in series with a 100Ω resistor. Then I connected the photodiode in series with a 300kΩ resistor to the ESP32-C3 board. I set the photodiode and led about 2cm apart. The python script `send` is used 
to send a message to be deciphered with the specified number of repetitions. For example, `./send 2 "hello world"` would send hello world twice in morse code. To decipher, you must run the commands, `idf.py build`, `idf.py flash`, `idf.py monitor` in that order. After running the last command, you send the message and it will print to the terminal. 

## Results
I was able to achieve ~8 characters per second with 100% accuracy. 