//
// Created by Vincent on 2024/2/4.
//

#include "XMPAddInterface.h"


int addXMPKV(cszstr * input_img_file_csz, cszstr * key_value_json_csz, cszstr * output_img_file_csz) {

    std::string kv_str = "";
    for (int i=0;i<key_value_json_csz->len;i++) {
        kv_str.push_back(key_value_json_csz->buf[i]);
    }

    std::map<std::string, std::string> meta_map;

    std::stringstream ss(kv_str);
    std::string token;

    int count=0;
    while (std::getline(ss, token, ';')) {
        size_t pos = token.find(':');
        std::string key = token.substr(0, pos);
        std::string value = token.substr(pos + 1);

        if (key == "" ||  value=="") {
            continue;
        }

        meta_map[key] = value;
        count++;
        if (count>100) {
            printf("key_value_json_csz error: count error!\n");
            break;
        }
    }

    if(count<=0) {
        return -1;
    }

    Exiv2::Image::UniquePtr image = Exiv2::ImageFactory::open(input_img_file_csz->buf, input_img_file_csz->len);

    image->readMetadata();

    Exiv2::XmpData xmpData;

    Exiv2::XmpProperties::registerNs("AIGCNamespace/", "aigc");

    std::map<std::string, std::string>::iterator iter;
    iter = meta_map.begin();
    while(iter != meta_map.end()) {
        std::string real_key = "Xmp.aigc." + iter->first;
        std::string value = iter->second;
        xmpData[real_key] = value;
        iter++;
    }

    image->setXmpData(xmpData);

    image->writeMetadata();

    size_t new_size = image->io().size();

    cszstr_resize(output_img_file_csz, new_size);

    image->io().read(output_img_file_csz->buf, new_size);

    return 0;

}

