
#pragma once

#include <iostream>

namespace FileTypes {
    bool yaz0Encode(std::istream& in, std::ostream& out);
    bool yaz0Decode(std::istream& in, std::ostream& out);
}