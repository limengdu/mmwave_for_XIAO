#ifndef Arduino_h
#define Arduino_h

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>

typedef uint8_t byte;
typedef uint8_t boolean;

#define HEX 16
#define DEC 10
#define HIGH 0x1
#define LOW 0x0

unsigned long millis();
void delay(unsigned long ms);

class String
{
public:
    String() {}
    String(const char *value) : _value(value ? value : "") {}
    String(int value) : _value(std::to_string(value)) {}
    String(const std::string &value) : _value(value) {}

    const char *c_str() const { return _value.c_str(); }
    operator std::string() const { return _value; }

    bool operator==(const char *other) const { return _value == other; }
    bool operator!=(const char *other) const { return _value != other; }
    bool operator==(const String &other) const { return _value == other._value; }

    friend String operator+(const char *lhs, const String &rhs)
    {
        return String(std::string(lhs) + rhs._value);
    }
    friend String operator+(const String &lhs, const String &rhs)
    {
        return String(lhs._value + rhs._value);
    }

private:
    std::string _value;
};

class Print
{
public:
    virtual ~Print() {}
    virtual size_t write(uint8_t) { return 1; }
    virtual size_t write(const uint8_t *buffer, size_t size)
    {
        for (size_t i = 0; i < size; i++)
        {
            write(buffer[i]);
        }
        return size;
    }
    size_t print(const char *) { return 0; }
    size_t print(char) { return 0; }
    size_t print(int, int = DEC) { return 0; }
    size_t print(unsigned int, int = DEC) { return 0; }
    size_t print(uint8_t, int = DEC) { return 0; }
    size_t println(const char *) { return 0; }
    size_t println(const String &) { return 0; }
    size_t println(int, int = DEC) { return 0; }
    size_t println() { return 0; }
};

class Stream : public Print
{
public:
    virtual int available() = 0;
    virtual int read() = 0;
};

class HardwareSerial : public Stream
{
public:
    int available() override { return 0; }
    int read() override { return -1; }
};

extern HardwareSerial Serial;

#endif
