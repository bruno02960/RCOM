#pragma once

extern int alarmFlag;
extern int fileDescriptor;

/**
 * Handles the alarm by printing a message
*/
void alarmHandler();

/**
  * Sets the alarm
   @param fd É PARA REMOVER
*/
void setAlarm(int fd);

/**
  * Stops the alarm
   @param fd É PARA REMOVER
*/
void stopAlarm(int fd);

/**
  * Sets a new VMIN on termios
   @param noChars new value for VMIN
*/
void setVMIN(int noChars);