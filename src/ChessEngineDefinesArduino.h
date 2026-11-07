#pragma once

#if defined(ARDUINO) || !defined(CHESS_ENGINE_NO_ARDUINO_FAKE)
#include <Arduino.h>
#else
#include <ctype.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string>
#include <chrono>
#include <iostream>
#include <thread>

class String {
 public:
    String() {}
    String(const char* text) :
        data(text ? text : "") {}
    String(char ch) :
        data(1, ch) {}
    String(int number) :
        data(std::to_string(number)) {}

    size_t length() const { return data.size(); }
    const char* c_str() const { return data.c_str(); }
    long toInt() const { return std::stol(data); }

    char charAt(unsigned int pos) const { return data[pos]; }
    char operator[](unsigned int pos) const { return data[pos]; }

    int indexOf(char ch) const { return indexOf(ch, 0); }
    int indexOf(char ch, unsigned int begin) const {
        const auto pos = data.find(ch, begin);
        return pos == std::string::npos ? -1 : static_cast<int>(pos);
    }
    int indexOf(const char* text) const {
        const auto pos = data.find(text);
        return pos == std::string::npos ? -1 : static_cast<int>(pos);
    }

    void trim() {
        const auto begin = data.find_first_not_of(" \t\r\n");
        if (begin == std::string::npos) {
            data.clear();
            return;
        }
        const auto end = data.find_last_not_of(" \t\r\n");
        data = data.substr(begin, end - begin + 1);
    }

    String substring(unsigned int begin) const { return data.substr(begin).c_str(); }
    String substring(unsigned int begin, unsigned int end) const { return data.substr(begin, end - begin).c_str(); }

    String& operator+=(char ch) {
        data += ch;
        return *this;
    }
    String& operator+=(const String& other) {
        data += other.data;
        return *this;
    }
    String operator+(char ch) const {
        String result(*this);
        result += ch;
        return result;
    }
    String operator+(const String& other) const {
        String result(*this);
        result += other;
        return result;
    }
    bool operator==(const char* text) const { return data == text; }

 private:
    std::string data;
};

class NativeSerial {
 public:
    int available() const {
        const std::streamsize n = std::cin.rdbuf()->in_avail();
        return n > 0 ? static_cast<int>(n) : 0;
    }

    int read() {
        if (!available()) {
            return -1;
        }
        const int ch = std::cin.get();
        return ch == EOF ? -1 : ch;
    }

    String readString() {
        std::string line;
        if (!std::getline(std::cin, line)) {
            return String();
        }
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        return String(line.c_str());
    }

    void println(const char* text) {
        std::cout << (text ? text : "") << '\n';
        std::cout.flush();
    }
};

inline NativeSerial& nativeSerial() {
    static NativeSerial serial;
    return serial;
}
#define Serial nativeSerial()

inline unsigned long millis() {
    static const auto start = std::chrono::steady_clock::now();
    const auto elapsed = std::chrono::steady_clock::now() - start;
    return static_cast<unsigned long>(std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count());
}

inline void delay(unsigned long ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

inline void delayMicroseconds(unsigned int us) {
    std::this_thread::sleep_for(std::chrono::microseconds(us));
}
#endif
