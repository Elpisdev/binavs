#include "binavs.h"
#include <iostream>

int main(int argc, char* argv[])
{
    if (argc < 4)
    {
        std::cout << "Usage: " << argv[0] << " [bin | xml] input_file output_file" << std::endl;
        return 1;
    }

    if (std::string(argv[1]) == "bin")
    {
        if (!xml_to_kbin(argv[2], argv[3]))
            return 1;
    }
    else if (std::string(argv[1]) == "xml")
    {
        if (!kbin_to_xml(argv[2], argv[3]))
            return 1;
    }
    else
    {
        std::cout << "Invalid command: " << argv[1] << std::endl;
        return 1;
    }

    return 0;
}