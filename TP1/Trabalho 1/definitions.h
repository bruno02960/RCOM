#pragma once

#define ERROR -1
#define TRANSMITTER 0
#define RECEIVER 1

#define COMMAND_SIZE 5

/**
S and U frames
*/
#define FLAG 0x7E //First and last byte

#define ADDR_S 0x03 //Sender perspective: commands sent by Sender and sent by Receiver(or received by Sender)
#define ADDR_R 0x01 //Receiver perspective: commands sent by Receiver and sent by Sender(or received by Receiver)

#define CTRL_SET 0x03 //set up
#define CTRL_DISC 0x0B //disconnect
#define CTRL_UA 0x07 //acknowledgement
#define CTRL_RR 0x05 //receiver ready
#define CTRL_REJ 0x01 //reject



