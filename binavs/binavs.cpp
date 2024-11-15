#include "binavs.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <vector>
#include <cstring>
#include <cstdlib>

#include <windows.h>

void swapByteOrder(unsigned short& us)
{
    us = (us >> 8) | (us << 8);
}

void swapByteOrder(unsigned int& ui)
{
    ui = (ui >> 24) |
        ((ui << 8) & 0x00FF0000) |
        ((ui >> 8) & 0x0000FF00) |
        (ui << 24);
}

void swapByteOrder(unsigned long long& ull)
{
    ull = (ull >> 56) |
        ((ull << 40) & 0x00FF000000000000) |
        ((ull << 24) & 0x0000FF0000000000) |
        ((ull << 8) & 0x000000FF00000000) |
        ((ull >> 8) & 0x00000000FF000000) |
        ((ull >> 24) & 0x0000000000FF0000) |
        ((ull >> 40) & 0x000000000000FF00) |
        (ull << 56);
}

ByteBufferWriter::ByteBufferWriter()
{
    capacity = 8192;
    buffer = new unsigned char[capacity];
    offset = 0;
}

ByteBufferWriter::~ByteBufferWriter()
{
    if (buffer)
        delete[] buffer;
}

void ByteBufferWriter::ensure(int size)
{
    if (offset + size >= capacity)
    {
        int new_capacity = capacity * 2;
        while (offset + size >= new_capacity)
            new_capacity *= 2;

        unsigned char* new_buffer = new unsigned char[new_capacity];
        memcpy(new_buffer, buffer, capacity);
        delete[] buffer;
        buffer = new_buffer;
        capacity = new_capacity;
    }
}

void ByteBufferWriter::append_u8(unsigned char data)
{
    ensure(1);
    buffer[offset++] = data;
}

void ByteBufferWriter::append_s32(int data)
{
    ensure(4);
    unsigned int prep_data = (unsigned int)data;
    swapByteOrder(prep_data);
    memcpy(&buffer[offset], &prep_data, 4);
    offset += 4;
}

void ByteBufferWriter::append_u32(unsigned int data)
{
    ensure(4);
    unsigned int prep_data = data;
    swapByteOrder(prep_data);
    memcpy(&buffer[offset], &prep_data, 4);
    offset += 4;
}

void ByteBufferWriter::append_bytes(const unsigned char* data, int len)
{
    ensure(len);
    memcpy(&buffer[offset], data, len);
    offset += len;
}

void ByteBufferWriter::append(std::string value, int nodeType)
{
    if (nodeType == 2 || nodeType == 16 || nodeType == 26 || nodeType == 36 || nodeType == 48 || nodeType == 52 || nodeType == 53 || nodeType == 54 || nodeType == 55 || nodeType == 56)
    {
        signed char cc = (signed char)std::stoi(value);
        append_bytes((const unsigned char*)&cc, 1);
    }
    else if (nodeType == 3 || nodeType == 17 || nodeType == 27 || nodeType == 37 || nodeType == 49)
    {
        unsigned char cc = (unsigned char)std::stoul(value);
        append_bytes((const unsigned char*)&cc, 1);
    }
    else if (nodeType == 4 || nodeType == 18 || nodeType == 28 || nodeType == 38 || nodeType == 50)
    {
        short cc = (short)std::stoi(value);
        unsigned short cd;
        memcpy(&cd, &cc, 2);
        swapByteOrder(cd);
        append_bytes((const unsigned char*)&cd, 2);
    }
    else if (nodeType == 5 || nodeType == 19 || nodeType == 29 || nodeType == 39 || nodeType == 51)
    {
        unsigned short cc = (unsigned short)std::stoul(value);
        swapByteOrder(cc);
        append_bytes((const unsigned char*)&cc, 2);
    }
    else if (nodeType == 6 || nodeType == 20 || nodeType == 30 || nodeType == 40)
    {
        int cc = std::stoi(value);
        unsigned int cd;
        memcpy(&cd, &cc, 4);
        swapByteOrder(cd);
        append_bytes((const unsigned char*)&cd, 4);
    }
    else if (nodeType == 7 || nodeType == 13 || nodeType == 21 || nodeType == 31 || nodeType == 41)
    {
        unsigned int cc = (unsigned int)std::stoul(value);
        swapByteOrder(cc);
        append_bytes((const unsigned char*)&cc, 4);
    }
    else if (nodeType == 8 || nodeType == 22 || nodeType == 32 || nodeType == 42)
    {
        long long cc = std::stoll(value);
        unsigned long long cd;
        memcpy(&cd, &cc, 8);
        swapByteOrder(cd);
        append_bytes((const unsigned char*)&cd, 8);
    }
    else if (nodeType == 9 || nodeType == 23 || nodeType == 33 || nodeType == 43)
    {
        unsigned long long cc = std::stoull(value);
        swapByteOrder(cc);
        append_bytes((const unsigned char*)&cc, 8);
    }
    else if (nodeType == 14 || nodeType == 24 || nodeType == 34 || nodeType == 44)
    {
        float cc = std::stof(value);
        unsigned int cd;
        memcpy(&cd, &cc, 4);
        swapByteOrder(cd);
        append_bytes((const unsigned char*)&cd, 4);
    }
    else if (nodeType == 15 || nodeType == 25 || nodeType == 35 || nodeType == 45)
    {
        double cc = std::stod(value);
        unsigned long long cd;
        memcpy(&cd, &cc, 8);
        swapByteOrder(cd);
        append_bytes((const unsigned char*)&cd, 8);
    }
    else if (nodeType == 12)
    {
        std::stringstream ss(value);
        int octet;
        char dot;
        for (int i = 0; i < 4; ++i)
        {
            ss >> octet;
            append_u8((unsigned char)octet);
            if (i < 3)
                ss >> dot;
        }
    }
    else
    {
    }
}

void ByteBufferWriter::realign_writes(int size)
{
    ensure(size);
    while (offset % size != 0)
        append_u8(0);
}

std::ostream& operator<<(std::ostream& stream, const ByteBufferWriter& bb)
{
    stream.write((char*)(bb.buffer), bb.offset);
    return stream;
}

ByteBufferReader::ByteBufferReader(unsigned char* buffer, int length)
{
    this->buffer = buffer;
    this->length = length;
    this->offset = 0;
}

ByteBufferReader::ByteBufferReader(unsigned char* buffer, int length, int offset)
{
    this->buffer = buffer;
    this->length = length;
    this->offset = offset;
}

bool ByteBufferReader::hasData()
{
    return offset < length;
}

unsigned char ByteBufferReader::peek_u8()
{
    if (hasData())
        return buffer[offset];
    return 0;
}

unsigned char ByteBufferReader::get_u8()
{
    if (hasData())
    {
        return buffer[offset++];
    }
    return 0;
}

unsigned short ByteBufferReader::get_u16()
{
    if (hasData())
    {
        unsigned short result;
        memcpy(&result, &buffer[offset], 2);
        offset += 2;
        swapByteOrder(result);
        return result;
    }
    return 0;
}

short ByteBufferReader::get_s16()
{
    return (short)get_u16();
}

unsigned int ByteBufferReader::get_u32()
{
    if (hasData())
    {
        unsigned int result;
        memcpy(&result, &buffer[offset], 4);
        offset += 4;
        swapByteOrder(result);
        return result;
    }
    return 0;
}

int ByteBufferReader::get_s32()
{
    return (int)get_u32();
}

unsigned long long ByteBufferReader::get_u64()
{
    if (hasData())
    {
        unsigned long long result;
        memcpy(&result, &buffer[offset], 8);
        offset += 8;
        swapByteOrder(result);
        return result;
    }
    return 0;
}

long long ByteBufferReader::get_s64()
{
    return (long long)get_u64();
}

float ByteBufferReader::get_float()
{
    unsigned int temp;
    memcpy(&temp, &buffer[offset], 4);
    swapByteOrder(temp);
    float result;
    memcpy(&result, &temp, 4);
    offset += 4;
    return result;
}

double ByteBufferReader::get_double()
{
    unsigned long long temp;
    memcpy(&temp, &buffer[offset], 8);
    swapByteOrder(temp);
    double result;
    memcpy(&result, &temp, 8);
    offset += 8;
    return result;
}

void ByteBufferReader::get_bytes(unsigned char* bits, int count)
{
    memcpy(bits, &buffer[offset], count);
    offset += count;
}

void ByteBufferReader::realign_reads(int size)
{
    while (offset % size != 0)
        offset += 1;
}

void shift_left_byte_array(unsigned char* bits, int length, int shift)
{
    if (shift > 0)
    {
        while (shift--) {
            int carry = 0;
            for (int i = length - 1; i >= 0; --i) {
                int next = (bits[i] & 0x80) ? 0x01 : 0;
                bits[i] = carry | (bits[i] << 1);
                carry = next;
            }
        }
    }
}

void shift_right_byte_array(unsigned char* bits, int length, int shift)
{
    if (shift > 0)
    {
        while (shift--) {
            int carry = 0;
            for (int i = 0; i <= length - 1; ++i) {
                int next = (bits[i] & 1) ? 0x80 : 0;
                bits[i] = carry | (bits[i] >> 1);
                carry = next;
            }
        }
    }
}

size_t split(const std::string& txt, std::vector<std::string>& strs, char ch)
{
    size_t pos = txt.find(ch);
    size_t initialPos = 0;
    strs.clear();

    while (pos != std::string::npos) {
        strs.push_back(txt.substr(initialPos, pos - initialPos));
        initialPos = pos + 1;
        pos = txt.find(ch, initialPos);
    }

    strs.push_back(txt.substr(initialPos, txt.size() - initialPos));

    return strs.size();
}

std::string utf8_to_cp932(const char* encodedStr)
{
    std::string cp932_string;

    int strSize = lstrlenA(encodedStr);
    int wideCharSize = MultiByteToWideChar(CP_UTF8, 0, encodedStr, strSize, NULL, 0);
    wchar_t* unicodeStr = new wchar_t[wideCharSize + 1];

    MultiByteToWideChar(CP_UTF8, 0, encodedStr, strSize, unicodeStr, wideCharSize);
    unicodeStr[wideCharSize] = 0;

    int cp932Size = WideCharToMultiByte(932, 0, unicodeStr, wideCharSize, NULL, 0, NULL, NULL);
    char* buffer = new char[cp932Size + 1];
    WideCharToMultiByte(932, 0, unicodeStr, wideCharSize, buffer, cp932Size, NULL, NULL);
    buffer[cp932Size] = 0;

    cp932_string = buffer;

    delete[] unicodeStr;
    delete[] buffer;

    return cp932_string;
}

std::string cp932_to_utf8(const char* encodedStr, int encoding_key)
{
    std::string utf8_string;

    int strSize = lstrlenA(encodedStr);

    UINT codePage = 932;
    if (encoding_key == 0x00 || encoding_key == 0x80)
        codePage = 932; // CP932
    else if (encoding_key == 0x60)
        codePage = 20932; // EUC-JP
    else if (encoding_key == 0x40)
        codePage = 28591; // ISO-8859-1

    int wideCharSize = MultiByteToWideChar(codePage, 0, encodedStr, strSize, NULL, 0);
    wchar_t* unicodeStr = new wchar_t[wideCharSize + 1];

    MultiByteToWideChar(codePage, 0, encodedStr, strSize, unicodeStr, wideCharSize);
    unicodeStr[wideCharSize] = 0;

    int utf8Size = WideCharToMultiByte(CP_UTF8, 0, unicodeStr, wideCharSize, NULL, 0, NULL, NULL);
    char* buffer = new char[utf8Size + 1];
    WideCharToMultiByte(CP_UTF8, 0, unicodeStr, wideCharSize, buffer, utf8Size, NULL, NULL);
    buffer[utf8Size] = 0;

    utf8_string = buffer;

    delete[] unicodeStr;
    delete[] buffer;

    return utf8_string;
}

void pack_sixbit(const char* name, ByteBufferWriter& byteBuf)
{
    const char* charmap = "0123456789:ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz";
    int length = strlen(name);

    int padding = 8 - (length * 6 % 8);

    if (padding == 8)
        padding = 0;

    int length_bits = length * 6;
    int length_bytes = (length_bits + 7) / 8;

    unsigned char* bits = new unsigned char[length_bytes];
    memset(bits, 0, length_bytes);

    for (int k = 0; k < length; k++)
    {
        unsigned char char_index = 0;
        const char* pos = strchr(charmap, name[k]);
        if (pos)
            char_index = (unsigned char)(pos - charmap);

        shift_left_byte_array(bits, length_bytes, 6);
        bits[length_bytes - 1] |= char_index;
    }

    shift_left_byte_array(bits, length_bytes, padding);

    byteBuf.append_u8(length);
    for (int k = 0; k < length_bytes; k++)
    {
        byteBuf.append_u8(bits[k]);
    }

    delete[] bits;
}

std::string unpack_sixbit(ByteBufferReader& byteBuf)
{
    const char* charmap = "0123456789:ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz";
    int k = 0;

    std::string result = "";

    unsigned char length = byteBuf.get_u8();

    unsigned int length_bits = length * 6;
    unsigned int length_bytes = (length_bits + 7) / 8;
    unsigned int padding = 8 - (length_bits % 8);
    if (padding == 8)
        padding = 0;

    unsigned char* bits = new unsigned char[length_bytes];

    byteBuf.get_bytes(bits, length_bytes);

    shift_right_byte_array(bits, length_bytes, padding);

    for (k = 0; k < length; k++)
    {
        unsigned char char_index = bits[length_bytes - 1] & 0x3F;
        result = charmap[char_index] + result;
        shift_right_byte_array(bits, length_bytes, 6);
    }

    delete[] bits;
    return result;
}

void append_node_name(ByteBufferWriter& nodeBuf, const char* name, unsigned char compressed)
{
    if (compressed)
    {
        pack_sixbit(name, nodeBuf);
    }
    else
    {
        int len = (int)strlen(name);
        nodeBuf.append_u8((len - 1) | 64);
        nodeBuf.append_bytes((const unsigned char*)name, len);
    }
}

int calcsize(int nodeType)
{
    switch (nodeType)
    {
    case 2:
    case 3:
    case 10:
    case 11:
    case 16:
    case 17:
    case 26:
    case 27:
    case 36:
    case 37:
    case 48:
    case 49:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
        return 1;
    case 4:
    case 5:
    case 18:
    case 19:
    case 28:
    case 29:
    case 38:
    case 39:
    case 50:
    case 51:
        return 2;
    case 6:
    case 7:
    case 12:
    case 13:
    case 14:
    case 20:
    case 21:
    case 24:
    case 30:
    case 31:
    case 34:
    case 40:
    case 41:
    case 44:
        return 4;
    case 8:
    case 9:
    case 15:
    case 22:
    case 23:
    case 25:
    case 32:
    case 33:
    case 35:
    case 42:
    case 43:
    case 45:
        return 8;
    default:
        return 0;
    }
}

void node_to_binary(unsigned char compressed, const pugi::xml_node& node, ByteBufferWriter& nodeBuf, ByteBufferWriter& dataBuf, int& byteOffset, int& wordOffset)
{
    int nodeType = 1;
    int varCount = -1;
    pugi::xml_attribute typeAttr = node.attribute("__type");

    if (!typeAttr)
    {
        if (node.value())
        {
            if (strlen(node.value()))
            {
                nodeType = XML_TYPES_STRING;
            }
        }
    }
    else
    {
        std::string typeStr = typeAttr.value();
        if (typeStr == "s8")
        {
            nodeType = 2;
            varCount = 1;
        }
        else if (typeStr == "u8")
        {
            nodeType = 3;
            varCount = 1;
        }
        else if (typeStr == "s16")
        {
            nodeType = 4;
            varCount = 1;
        }
        else if (typeStr == "u16")
        {
            nodeType = 5;
            varCount = 1;
        }
        else if (typeStr == "s32")
        {
            nodeType = 6;
            varCount = 1;
        }
        else if (typeStr == "u32")
        {
            nodeType = 7;
            varCount = 1;
        }
        else if (typeStr == "s64")
        {
            nodeType = 8;
            varCount = 1;
        }
        else if (typeStr == "u64")
        {
            nodeType = 9;
            varCount = 1;
        }
        else if (typeStr == "bin" || typeStr == "binary")
        {
            nodeType = XML_TYPES_BINARY;
            varCount = -1;
        }
        else if (typeStr == "str" || typeStr == "string")
        {
            nodeType = XML_TYPES_STRING;
            varCount = -1;
        }
        else if (typeStr == "ip4")
        {
            nodeType = 12;
            varCount = 1;
        }
        else if (typeStr == "time")
        {
            nodeType = 13;
            varCount = 1;
        }
        else if (typeStr == "float")
        {
            nodeType = 14;
            varCount = 1;
        }
        else if (typeStr == "double")
        {
            nodeType = 15;
            varCount = 1;
        }
        else if (typeStr == "2s8")
        {
            nodeType = 16;
            varCount = 2;
        }
        else if (typeStr == "2u8")
        {
            nodeType = 17;
            varCount = 2;
        }
        else if (typeStr == "2s16")
        {
            nodeType = 18;
            varCount = 2;
        }
        else if (typeStr == "2u16")
        {
            nodeType = 19;
            varCount = 2;
        }
        else if (typeStr == "2s32")
        {
            nodeType = 20;
            varCount = 2;
        }
        else if (typeStr == "2u32")
        {
            nodeType = 21;
            varCount = 2;
        }
        else if (typeStr == "2s64" || typeStr == "vs64")
        {
            nodeType = 22;
            varCount = 2;
        }
        else if (typeStr == "2u64" || typeStr == "vu64")
        {
            nodeType = 23;
            varCount = 2;
        }
        else if (typeStr == "2f")
        {
            nodeType = 24;
            varCount = 2;
        }
        else if (typeStr == "2d" || typeStr == "vd")
        {
            nodeType = 25;
            varCount = 2;
        }
        else if (typeStr == "3s8")
        {
            nodeType = 26;
            varCount = 3;
        }
        else if (typeStr == "3u8")
        {
            nodeType = 27;
            varCount = 3;
        }
        else if (typeStr == "3s16")
        {
            nodeType = 28;
            varCount = 3;
        }
        else if (typeStr == "3u16")
        {
            nodeType = 29;
            varCount = 3;
        }
        else if (typeStr == "3s32")
        {
            nodeType = 30;
            varCount = 3;
        }
        else if (typeStr == "3u32")
        {
            nodeType = 31;
            varCount = 3;
        }
        else if (typeStr == "3s64")
        {
            nodeType = 32;
            varCount = 3;
        }
        else if (typeStr == "3u64")
        {
            nodeType = 33;
            varCount = 3;
        }
        else if (typeStr == "3f")
        {
            nodeType = 34;
            varCount = 3;
        }
        else if (typeStr == "3d")
        {
            nodeType = 35;
            varCount = 3;
        }
        else if (typeStr == "4s8")
        {
            nodeType = 36;
            varCount = 4;
        }
        else if (typeStr == "4u8")
        {
            nodeType = 37;
            varCount = 4;
        }
        else if (typeStr == "4s16")
        {
            nodeType = 38;
            varCount = 4;
        }
        else if (typeStr == "4u16")
        {
            nodeType = 39;
            varCount = 4;
        }
        else if (typeStr == "4s32" || typeStr == "vs32")
        {
            nodeType = 40;
            varCount = 4;
        }
        else if (typeStr == "4u32" || typeStr == "vu32")
        {
            nodeType = 41;
            varCount = 4;
        }
        else if (typeStr == "4s64")
        {
            nodeType = 42;
            varCount = 4;
        }
        else if (typeStr == "4u64")
        {
            nodeType = 43;
            varCount = 4;
        }
        else if (typeStr == "4f" || typeStr == "vf")
        {
            nodeType = 44;
            varCount = 4;
        }
        else if (typeStr == "4d")
        {
            nodeType = 45;
            varCount = 4;
        }
        else if (typeStr == "vs8")
        {
            nodeType = 48;
            varCount = 16;
        }
        else if (typeStr == "vu8")
        {
            nodeType = 49;
            varCount = 16;
        }
        else if (typeStr == "vs16")
        {
            nodeType = 50;
            varCount = 8;
        }
        else if (typeStr == "vu16")
        {
            nodeType = 51;
            varCount = 8;
        }
        else if (typeStr == "b" || typeStr == "bool")
        {
            nodeType = 52;
            varCount = 1;
        }
        else if (typeStr == "2b")
        {
            nodeType = 53;
            varCount = 2;
        }
        else if (typeStr == "3b")
        {
            nodeType = 54;
            varCount = 3;
        }
        else if (typeStr == "4b")
        {
            nodeType = 55;
            varCount = 4;
        }
        else if (typeStr == "vb")
        {
            nodeType = 56;
            varCount = 16;
        }
        else
        {
            // unrecognized type
        }
    }

    int isArray = 0;
    pugi::xml_attribute countAttr = node.attribute("__count");
    int count = 1;
    if (countAttr)
    {
        count = atoi(node.attribute("__count").value());
        isArray = 64;  // bit position for array flag
    }

    nodeBuf.append_u8(nodeType | isArray);
    std::string node_name = node.name();
    append_node_name(nodeBuf, node_name.c_str(), compressed);

    if (nodeType != 1)
    {
        std::string val = node.first_child().value();
        std::vector<std::string> values;
        split(val, values, ' ');

        if (isArray || varCount == -1)
        {
            if (nodeType == XML_TYPES_BINARY)
            {
                dataBuf.append_u32(val.size() / 2);
                for (size_t i = 0; i < val.size(); i += 2)
                {
                    std::string hexchar = val.substr(i, 2);
                    unsigned char cc = (unsigned char)strtol(hexchar.c_str(), NULL, 16);
                    dataBuf.append_u8(cc);
                }
                dataBuf.realign_writes(4);
            }
            else if (nodeType == XML_TYPES_STRING)
            {
                std::string val2 = utf8_to_cp932(val.c_str());
                dataBuf.append_u32(val2.size() + 1);
                dataBuf.append_bytes((const unsigned char*)val2.c_str(), val2.size() + 1);
                dataBuf.realign_writes(4);
            }
            else
            {
                dataBuf.append_u32(values.size() * calcsize(nodeType));
                for (size_t i = 0; i < values.size(); i++)
                {
                    dataBuf.append(values[i], nodeType);
                }
                dataBuf.realign_writes(4);
            }
        }
        else
        {
            if (byteOffset % 4 == 0)
            {
                byteOffset = dataBuf.offset;
            }
            if (wordOffset % 4 == 0)
            {
                wordOffset = dataBuf.offset;
            }

            int size = calcsize(nodeType) * varCount;

            if (size == 1)
            {
                if (byteOffset % 4 == 0)
                    dataBuf.append_u32(0);
                int offset = dataBuf.offset;
                dataBuf.offset = byteOffset;
                for (size_t i = 0; i < values.size(); i++)
                {
                    dataBuf.append(values[i], nodeType);
                }
                byteOffset = dataBuf.offset;
                dataBuf.offset = offset;
            }
            else if (size == 2)
            {
                if (wordOffset % 4 == 0)
                    dataBuf.append_u32(0);
                int offset = dataBuf.offset;
                dataBuf.offset = wordOffset;
                for (size_t i = 0; i < values.size(); i++)
                {
                    dataBuf.append(values[i], nodeType);
                }
                wordOffset = dataBuf.offset;
                dataBuf.offset = offset;
            }
            else
            {
                for (size_t i = 0; i < values.size(); i++)
                {
                    dataBuf.append(values[i], nodeType);
                }
                dataBuf.realign_writes(4);
            }
        }
    }

    std::vector<std::string> sorted_attr_names;
    for (pugi::xml_attribute attr : node.attributes())
    {
        sorted_attr_names.push_back(attr.name());
    }
    std::sort(sorted_attr_names.begin(), sorted_attr_names.end());

    for (std::string attr_name : sorted_attr_names)
    {
        if (attr_name != "__type" && attr_name != "__size" && attr_name != "__count")
        {
            std::string attr_value = utf8_to_cp932(node.attribute(attr_name.c_str()).value());
            dataBuf.append_s32(attr_value.size() + 1);
            dataBuf.append_bytes((const unsigned char*)attr_value.c_str(), attr_value.size() + 1);
            dataBuf.realign_writes(4);

            nodeBuf.append_u8(XML_TYPES_ATTRIBUTE);
            append_node_name(nodeBuf, attr_name.c_str(), compressed);
        }
    }

    for (pugi::xml_node child_node : node.children())
    {
        if (child_node.type() == pugi::node_element)
        {
            node_to_binary(compressed, child_node, nodeBuf, dataBuf, byteOffset, wordOffset);
        }
    }

    nodeBuf.append_u8(XML_TYPES_NODE_END | 64);
}

void to_binary(pugi::xml_document& doc, pugi::xml_encoding xml_encoding, std::ofstream& output)
{
    ByteBufferWriter header;
    unsigned char bin_encoding = 0x80;
    unsigned char compressed = 1;

    header.append_u8(SIGNATURE);
    if (compressed)
        header.append_u8(SIG_COMPRESSED);
    else
        header.append_u8(SIG_UNCOMPRESSED);

    header.append_u8(bin_encoding);
    header.append_u8(0xFF ^ bin_encoding);

    ByteBufferWriter nodeBuf;
    ByteBufferWriter dataBuf;

    int byteOffset = 0;
    int wordOffset = 0;

    node_to_binary(compressed, doc.first_child(), nodeBuf, dataBuf, byteOffset, wordOffset);

    nodeBuf.append_u8(XML_TYPES_END_SECTION | 64);
    nodeBuf.realign_writes(4);
    header.append_u32((unsigned int)nodeBuf.offset);
    int dataSize = dataBuf.offset;
    nodeBuf.append_u32(dataSize);

    output << header;
    output << nodeBuf;
    output << dataBuf;
}

void grab(std::stringstream& ss, int nodeType, ByteBufferReader& dataBuf)
{
    int b1, b2, b3, b4;
    unsigned char cc;

    switch (nodeType)
    {
    case 2:
    case 16:
    case 26:
    case 36:
    case 48:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
        cc = dataBuf.get_u8();
        b1 = (signed char)cc;
        ss << b1;
        break;
    case 3:
    case 17:
    case 27:
    case 37:
    case 49:
        cc = dataBuf.get_u8();
        b1 = cc;
        ss << b1;
        break;
    case 4:
    case 18:
    case 28:
    case 38:
    case 50:
        ss << dataBuf.get_s16();
        break;
    case 5:
    case 19:
    case 29:
    case 39:
    case 51:
        ss << dataBuf.get_u16();
        break;
    case 6:
    case 20:
    case 30:
    case 40:
        ss << dataBuf.get_s32();
        break;
    case 7:
    case 21:
    case 31:
    case 41:
        ss << dataBuf.get_u32();
        break;
    case 8:
    case 22:
    case 32:
    case 42:
        ss << dataBuf.get_s64();
        break;
    case 9:
    case 23:
    case 33:
    case 43:
        ss << dataBuf.get_u64();
        break;
    case 12:
        b1 = dataBuf.get_u8();
        b2 = dataBuf.get_u8();
        b3 = dataBuf.get_u8();
        b4 = dataBuf.get_u8();
        ss << b1 << "." << b2 << "." << b3 << "." << b4;
        break;
    case 14:
    case 24:
    case 34:
    case 44:
    {
        float fval = dataBuf.get_float();
        ss << std::fixed << std::setprecision(6) << fval;
        break;
    }
    case 15:
    case 25:
    case 35:
    case 45:
    {
        double dval = dataBuf.get_double();
        ss << std::fixed << std::setprecision(6) << dval;
        break;
    }
    default:
        break;
    }
}

void grab_aligned(std::stringstream& ss, int nodeType, int count, ByteBufferReader& dataBuf, ByteBufferReader& dataByteBuf, ByteBufferReader& dataWordBuf)
{
    if (dataByteBuf.offset % 4 == 0)
        dataByteBuf.offset = dataBuf.offset;
    if (dataWordBuf.offset % 4 == 0)
        dataWordBuf.offset = dataBuf.offset;

    int size = calcsize(nodeType) * count;

    if (size == 1)
    {
        for (int i = 0; i < count; i++)
        {
            if (ss.tellp() && nodeType != XML_TYPES_BINARY)
                ss << " ";
            grab(ss, nodeType, dataByteBuf);
        }
    }
    else if (size == 2)
    {
        for (int i = 0; i < count; i++)
        {
            if (ss.tellp() && nodeType != XML_TYPES_BINARY)
                ss << " ";
            grab(ss, nodeType, dataWordBuf);
        }
    }
    else
    {
        for (int i = 0; i < count; i++)
        {
            if (ss.tellp() && nodeType != XML_TYPES_BINARY)
                ss << " ";
            grab(ss, nodeType, dataBuf);
        }
        dataBuf.realign_reads(4);
    }
    int trailing = max(dataByteBuf.offset, dataWordBuf.offset);

    if (dataBuf.offset < trailing)
    {
        dataBuf.offset = trailing;
        dataBuf.realign_reads(4);
    }
}

std::string apply_encoding(const char* encodedStr, int encoding_key)
{
    // leave as is
    if (encoding_key == 0x20 || encoding_key == 0xA0)
    {
        return std::string(encodedStr);
    }
    else
    {
        return cp932_to_utf8(encodedStr, encoding_key);
    }
}

void from_binary(unsigned char* buffer, int fileSize, std::ostream& result_stream)
{
    bool compressed;
    unsigned int dataSize;
    pugi::xml_document doc;

    unsigned char signature;
    unsigned char compress;
    unsigned char encoding_key;
    unsigned int nodeEnd;

    ByteBufferReader nodeBuf(buffer, fileSize);

    pugi::xml_node decl = doc.prepend_child(pugi::node_declaration);
    decl.append_attribute("version").set_value("1.0");
    decl.append_attribute("encoding").set_value("UTF-8");

    pugi::xml_node node;
    bool addFirstNode = true;

    signature = nodeBuf.get_u8();
    if (signature == SIGNATURE)
    {
        compress = nodeBuf.get_u8();
        if (compress == SIG_COMPRESSED || compress == SIG_UNCOMPRESSED)
        {
            compressed = compress == SIG_COMPRESSED;

            encoding_key = nodeBuf.get_u8();

            if (encoding_key == (nodeBuf.get_u8() ^ 0xFF))
            {
                bool nodesLeft = true;

                nodeEnd = nodeBuf.get_u32() + 8;

                ByteBufferReader dataBuf(buffer, fileSize, nodeEnd);
                ByteBufferReader dataByteBuf(buffer, fileSize, nodeEnd);
                ByteBufferReader dataWordBuf(buffer, fileSize, nodeEnd);

                unsigned int dataSize = dataBuf.get_u32() + 8;

                while (nodesLeft && nodeBuf.hasData())
                {
                    std::string name = "";

                    while (nodeBuf.peek_u8() == 0) {
                        nodeBuf.get_u8();
                    }

                    unsigned char nodeType = nodeBuf.get_u8();
                    unsigned char isArray = nodeType & 64;
                    nodeType &= ~64;

                    if (nodeType != XML_TYPES_NODE_END && nodeType != XML_TYPES_END_SECTION)
                    {
                        if (compressed)
                        {
                            name = unpack_sixbit(nodeBuf);
                        }
                        else
                        {
                            int length = (nodeBuf.get_u8() & ~64) + 1;
                            for (int i = 0; i < length; i++)
                                name += (char)nodeBuf.get_u8();
                        }
                    }

                    bool skip = true;

                    if (nodeType == XML_TYPES_ATTRIBUTE)
                    {
                        int valueSize = dataBuf.get_s32();
                        unsigned char* valueBuffer = new unsigned char[valueSize + 1];
                        memset(valueBuffer, 0, valueSize + 1);
                        dataBuf.get_bytes(valueBuffer, valueSize);
                        dataBuf.realign_reads(4);

                        std::string attrValue = apply_encoding((const char*)valueBuffer, encoding_key);
                        node.append_attribute(name.c_str()).set_value(attrValue.c_str());

                        delete[] valueBuffer;
                    }
                    else if (nodeType == XML_TYPES_NODE_END)
                    {
                        if (node.parent())
                            node = node.parent();
                    }
                    else if (nodeType == XML_TYPES_END_SECTION)
                    {
                        nodesLeft = false;
                    }
                    else
                    {
                        skip = false;
                    }

                    if (skip)
                        continue;

                    pugi::xml_node child = addFirstNode ? doc.append_child(name.c_str()) : node.append_child(name.c_str());
                    addFirstNode = false;
                    node = child;

                    if (nodeType == XML_TYPES_NODE_START)
                        continue;

                    unsigned int varCount = 1;
                    unsigned int calcSize = 0;
                    std::string typeStr;

                    switch (nodeType)
                    {
                    case 2:
                        typeStr = "s8";
                        varCount = 1;
                        calcSize = 1;
                        break;
                    case 3:
                        typeStr = "u8";
                        varCount = 1;
                        calcSize = 1;
                        break;
                    case 4:
                        typeStr = "s16";
                        varCount = 1;
                        calcSize = 2;
                        break;
                    case 5:
                        typeStr = "u16";
                        varCount = 1;
                        calcSize = 2;
                        break;
                    case 6:
                        typeStr = "s32";
                        varCount = 1;
                        calcSize = 4;
                        break;
                    case 7:
                        typeStr = "u32";
                        varCount = 1;
                        calcSize = 4;
                        break;
                    case 8:
                        typeStr = "s64";
                        varCount = 1;
                        calcSize = 8;
                        break;
                    case 9:
                        typeStr = "u64";
                        varCount = 1;
                        calcSize = 8;
                        break;
                    case 10:
                        typeStr = "bin";
                        varCount = -1;
                        calcSize = 1;
                        break;
                    case 11:
                        typeStr = "str";
                        varCount = -1;
                        calcSize = 1;
                        break;
                    case 12:
                        typeStr = "ip4";
                        varCount = 1;
                        calcSize = 4;
                        break;
                    case 13:
                        typeStr = "time";
                        varCount = 1;
                        calcSize = 4;
                        break;
                    case 14:
                        typeStr = "float";
                        varCount = 1;
                        calcSize = 4;
                        break;
                    case 15:
                        typeStr = "double";
                        varCount = 1;
                        calcSize = 8;
                        break;
                    case 16:
                        typeStr = "2s8";
                        varCount = 2;
                        calcSize = 1;
                        break;
                    case 17:
                        typeStr = "2u8";
                        varCount = 2;
                        calcSize = 1;
                        break;
                    case 18:
                        typeStr = "2s16";
                        varCount = 2;
                        calcSize = 2;
                        break;
                    case 19:
                        typeStr = "2u16";
                        varCount = 2;
                        calcSize = 2;
                        break;
                    case 20:
                        typeStr = "2s32";
                        varCount = 2;
                        calcSize = 4;
                        break;
                    case 21:
                        typeStr = "2u32";
                        varCount = 2;
                        calcSize = 4;
                        break;
                    case 22:
                        typeStr = "2s64";
                        varCount = 2;
                        calcSize = 8;
                        break;
                    case 23:
                        typeStr = "2u64";
                        varCount = 2;
                        calcSize = 8;
                        break;
                    case 24:
                        typeStr = "2f";
                        varCount = 2;
                        calcSize = 4;
                        break;
                    case 25:
                        typeStr = "2d";
                        varCount = 2;
                        calcSize = 8;
                        break;
                    case 26:
                        typeStr = "3s8";
                        varCount = 3;
                        calcSize = 1;
                        break;
                    case 27:
                        typeStr = "3u8";
                        varCount = 3;
                        calcSize = 1;
                        break;
                    case 28:
                        typeStr = "3s16";
                        varCount = 3;
                        calcSize = 2;
                        break;
                    case 29:
                        typeStr = "3u16";
                        varCount = 3;
                        calcSize = 2;
                        break;
                    case 30:
                        typeStr = "3s32";
                        varCount = 3;
                        calcSize = 4;
                        break;
                    case 31:
                        typeStr = "3u32";
                        varCount = 3;
                        calcSize = 4;
                        break;
                    case 32:
                        typeStr = "3s64";
                        varCount = 3;
                        calcSize = 8;
                        break;
                    case 33:
                        typeStr = "3u64";
                        varCount = 3;
                        calcSize = 8;
                        break;
                    case 34:
                        typeStr = "3f";
                        varCount = 3;
                        calcSize = 4;
                        break;
                    case 35:
                        typeStr = "3d";
                        varCount = 3;
                        calcSize = 8;
                        break;
                    case 36:
                        typeStr = "4s8";
                        varCount = 4;
                        calcSize = 1;
                        break;
                    case 37:
                        typeStr = "4u8";
                        varCount = 4;
                        calcSize = 1;
                        break;
                    case 38:
                        typeStr = "4s16";
                        varCount = 4;
                        calcSize = 2;
                        break;
                    case 39:
                        typeStr = "4u16";
                        varCount = 4;
                        calcSize = 2;
                        break;
                    case 40:
                        typeStr = "4s32";
                        varCount = 4;
                        calcSize = 4;
                        break;
                    case 41:
                        typeStr = "4u32";
                        varCount = 4;
                        calcSize = 4;
                        break;
                    case 42:
                        typeStr = "4s64";
                        varCount = 4;
                        calcSize = 8;
                        break;
                    case 43:
                        typeStr = "4u64";
                        varCount = 4;
                        calcSize = 8;
                        break;
                    case 44:
                        typeStr = "4f";
                        varCount = 4;
                        calcSize = 4;
                        break;
                    case 45:
                        typeStr = "4d";
                        varCount = 4;
                        calcSize = 8;
                        break;
                    case 48:
                        typeStr = "vs8";
                        varCount = 16;
                        calcSize = 1;
                        break;
                    case 49:
                        typeStr = "vu8";
                        varCount = 16;
                        calcSize = 1;
                        break;
                    case 50:
                        typeStr = "vs16";
                        varCount = 8;
                        calcSize = 2;
                        break;
                    case 51:
                        typeStr = "vu16";
                        varCount = 8;
                        calcSize = 2;
                        break;
                    case 52:
                        typeStr = "bool";
                        varCount = 1;
                        calcSize = 1;
                        break;
                    case 53:
                        typeStr = "2b";
                        varCount = 2;
                        calcSize = 1;
                        break;
                    case 54:
                        typeStr = "3b";
                        varCount = 3;
                        calcSize = 1;
                        break;
                    case 55:
                        typeStr = "4b";
                        varCount = 4;
                        calcSize = 1;
                        break;
                    case 56:
                        typeStr = "vb";
                        varCount = 16;
                        calcSize = 1;
                        break;
                    default:
                        break;
                    }

                    node.append_attribute("__type").set_value(typeStr.c_str());

                    int arrayCount = 1;
                    if (varCount == -1)
                    {
                        varCount = dataBuf.get_u32();
                        isArray = 1;
                    }
                    else if (isArray)
                    {
                        std::stringstream ss;
                        arrayCount = dataBuf.get_u32() / (calcsize(nodeType) * varCount);
                        ss << arrayCount;
                        node.append_attribute("__count").set_value(ss.str().c_str());
                    }
                    unsigned int totalCount = arrayCount * varCount;

                    std::stringstream node_ss;
                    if (isArray)
                    {
                        if (nodeType != XML_TYPES_STRING)
                        {
                            for (unsigned int i = 0; i < totalCount; i++)
                            {
                                if (node_ss.tellp() && nodeType != XML_TYPES_BINARY)
                                    node_ss << " ";
                                grab(node_ss, nodeType, dataBuf);
                            }
                            dataBuf.realign_reads(4);
                        }
                        else
                        {
                            unsigned char* strBuffer = new unsigned char[totalCount + 1];
                            memset(strBuffer, 0, totalCount + 1);
                            dataBuf.get_bytes(strBuffer, totalCount);

                            node_ss << strBuffer;

                            dataBuf.realign_reads(4);
                            delete[] strBuffer;
                        }
                    }
                    else
                    {
                        grab_aligned(node_ss, nodeType, totalCount, dataBuf, dataByteBuf, dataWordBuf);
                    }

                    if (nodeType == XML_TYPES_BINARY)
                    {
                        std::stringstream ss;
                        ss << totalCount;
                        node.append_attribute("__size").set_value(ss.str().c_str());
                    }
                    else if (nodeType == XML_TYPES_STRING)
                    {
                        std::string data = node_ss.str();
                        node_ss.str("");
                        std::string utf8_str = apply_encoding(data.c_str(), encoding_key);
                        node_ss << utf8_str;
                    }

                    node.append_child(pugi::node_pcdata).set_value(node_ss.str().c_str());
                }
            }
        }
    }

    doc.save(result_stream, "  ");
}

bool xml_to_kbin(const char* xml_filename, const char* kbin_filename)
{
    std::ifstream input(xml_filename, std::ios_base::binary);
    if (!input)
    {
        std::cerr << "Error opening XML file: " << xml_filename << std::endl;
        return false;
    }

    pugi::xml_document doc;
    pugi::xml_parse_result parse_result = doc.load(input);

    std::ofstream output(kbin_filename, std::ios_base::binary);
    if (!output)
    {
        std::cerr << "Error opening output file: " << kbin_filename << std::endl;
        return false;
    }

    to_binary(doc, parse_result.encoding, output);

    output.close();
    input.close();

    return true;
}

bool kbin_to_xml(const char* kbin_filename, const char* xml_filename)
{
    std::ifstream input(kbin_filename, std::ios_base::binary);
    if (!input)
    {
        std::cerr << "Error opening KBin file: " << kbin_filename << std::endl;
        return false;
    }

    input.seekg(0, std::ios::end);
    unsigned int fileSize = input.tellg();
    input.seekg(0, std::ios::beg);

    unsigned char* buffer = new unsigned char[fileSize];
    input.read((char*)buffer, fileSize);

    std::stringstream xml;

    from_binary(buffer, fileSize, xml);

    if (xml_filename)
    {
        std::ofstream output(xml_filename, std::ios_base::binary);
        if (!output)
        {
            std::cerr << "Error opening output file: " << xml_filename << std::endl;
            delete[] buffer;
            return false;
        }
        output << xml.str();
        output.close();
    }
    else
    {
        std::cout << xml.str();
    }

    delete[] buffer;
    input.close();

    return true;
}