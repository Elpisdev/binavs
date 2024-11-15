#ifndef BINAVS_H
#define BINAVS_H

#include <string>
#include "pugixml.hpp"

#define SIGNATURE 0xA0
#define SIG_COMPRESSED 0x42
#define SIG_UNCOMPRESSED 0x45

#define XML_TYPES_NODE_START 1
#define XML_TYPES_NODE_END 190
#define XML_TYPES_END_SECTION 191

#define XML_TYPES_ATTRIBUTE 46
#define XML_TYPES_BINARY 10
#define XML_TYPES_STRING 11

void swapByteOrder(unsigned short& us);
void swapByteOrder(unsigned int& ui);
void swapByteOrder(unsigned long long& ull);

class ByteBufferWriter
{
private:
    unsigned char* buffer;
    int capacity;

    void ensure(int size);

public:
    int offset;

    ByteBufferWriter();
    ~ByteBufferWriter();

    void append_u8(unsigned char data);
    void append_s32(int data);
    void append_u32(unsigned int data);
    void append_bytes(const unsigned char* data, int len);
    void append(std::string value, int nodeType);
    void realign_writes(int size);

    friend std::ostream& operator<<(std::ostream& stream, const ByteBufferWriter& bb);
};

class ByteBufferReader
{
private:
    unsigned char* buffer;
    int length;

public:
    int offset;

    ByteBufferReader(unsigned char* buffer, int length);
    ByteBufferReader(unsigned char* buffer, int length, int offset);

    bool hasData();

    unsigned char peek_u8();
    unsigned char get_u8();
    unsigned short get_u16();
    short get_s16();
    unsigned int get_u32();
    int get_s32();
    unsigned long long get_u64();
    long long get_s64();
    float get_float();
    double get_double();
    void get_bytes(unsigned char* bits, int count);
    void realign_reads(int size);
};

bool xml_to_kbin(const char* xml_filename, const char* kbin_filename);
bool kbin_to_xml(const char* kbin_filename, const char* xml_filename);

#endif // BINAVS_H