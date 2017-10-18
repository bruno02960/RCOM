#pragma once

extern int alarmFlag;
extern int fileDescriptor;

void handerALRM();

void setAlarm(int fd);

void stopAlarm(int fd);
