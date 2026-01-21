#include "serialhelper.hpp"
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <cstring>
#include <iostream>
#include <string>

int openSerial(const char* device, int baud) {
    int fd = open(device, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd < 0) {
        std::cerr << "Failed to open serial device: " << device
                  << " (" << strerror(errno) << ")\n";
        return -1;
    }

    // Put into blocking mode after open (simplifies writes)
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags & ~O_NONBLOCK);

    termios tty{};
    if (tcgetattr(fd, &tty) != 0) {
        std::cerr << "tcgetattr failed: " << strerror(errno) << "\n";
        close(fd);
        return -1;
    }

    // Raw mode
    cfmakeraw(&tty);

    // 8N1
    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
    tty.c_cflag |= (CLOCAL | CREAD);
    tty.c_cflag &= ~(PARENB | CSTOPB | CRTSCTS);

    // No software flow control
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);

    // Read timeouts (not super important if you're mostly writing)
    tty.c_cc[VMIN]  = 0;
    tty.c_cc[VTIME] = 5; // 0.5s

    speed_t speed = B115200; // you asked for 115200
    cfsetispeed(&tty, speed);
    cfsetospeed(&tty, speed);

    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        std::cerr << "tcsetattr failed: " << strerror(errno) << "\n";
        close(fd);
        return -1;
    }

    // Some boards reset when serial opens; small delay helps
    usleep(1500 * 1000);

    return fd;
}

bool writeLine(int fd, const std::string& line) {
    const char* data = line.c_str();
    size_t len = line.size();
    size_t total = 0;

    while (total < len) {
        ssize_t w = write(fd, data + total, len - total);
        if (w <= 0) return false;
        total += (size_t)w;
    }

    tcdrain(fd); // wait until it’s actually transmitted
    return true;
}
