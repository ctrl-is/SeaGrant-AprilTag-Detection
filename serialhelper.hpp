#pragma once
#include <string>

int openSerial(const char* device, int baud);
bool writeLine(int fd, const std::string& line);
