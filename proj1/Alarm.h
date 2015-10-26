#pragma once

extern int alarmFired;

void alarmHandler(int signal);

void setAlarm();

void stopAlarm();
