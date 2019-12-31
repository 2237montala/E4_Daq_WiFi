#ifndef MAIN_H

#define MAIN_H
#include <Arduino.h>

void getNewFiles();
void handleRoot();
void sendFile();
bool getCMD(String& incomingCmd,int timeout);
bool waitForACK(int timeout);
void sendCmd(String cmd,boolean addEOL,bool printCMD);
void transferFileNames();
void transferFileData(String fileName);

#endif
