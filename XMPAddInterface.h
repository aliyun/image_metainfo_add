//
// Created by Vincent on 2024/2/4.
//

#ifndef USE_LIB_XMPADDINTERFACE_H
#define USE_LIB_XMPADDINTERFACE_H

#include "JnaBuffer.h"
#include <string>
#include <map>
#include <sstream>
#include "exiv2/exiv2.hpp"

#include <iostream>

extern "C" {

    int addXMPKV(cszstr * input_img_file_csz, cszstr * key_value_json_csz, cszstr * output_img_file_csz);
};


#endif //USE_LIB_XMPADDINTERFACE_H
