// xmpadd.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <fstream>
#include <iostream>
#include <vector>
#include "JnaBuffer.h"
#include "XMPAddInterface.h"

using namespace std;

void cszstr_assign_x(cszstr* str, const char* sz) {
  cszstr_resize(str, strlen(sz));
  memcpy(str->buf, sz, str->len);
}

std::vector<unsigned char> readFileIntoString_1(const char* filename) {
  std::ifstream file(filename, std::ios::binary);

  std::vector<unsigned char> a;
  if (!file.is_open()) {
    std::cerr << "无法打开文件: " << filename << std::endl;
    return a;
  }

  file.seekg(0, std::ios::end);
  std::streampos fileSize = file.tellg();
  file.seekg(0, std::ios::beg);

  std::vector<unsigned char> fileData(fileSize);

  if (file.read(reinterpret_cast<char*>(fileData.data()), fileSize)) {
  } else {
    std::cerr << "读取文件时发生错误。" << std::endl;
  }

  file.close();
  return fileData;
}

int main_insert(char* input_file_path, char* output_file_path, char* kv_str) {
  cszstr input_image_byte, output_image_byte, key_value_json_csz;
  cszstr_init(&input_image_byte);
  cszstr_init(&output_image_byte);
  cszstr_init(&key_value_json_csz);

  cszstr_assign_x(&key_value_json_csz, kv_str);

  std::vector<unsigned char> file_str = readFileIntoString_1(input_file_path);

  cszstr_alloc(&input_image_byte, file_str.size());

  input_image_byte.len = file_str.size();
  memcpy(input_image_byte.buf, (char*)file_str.data(), input_image_byte.len);

  clock_t t1, t2;
  t1 = clock();

  int r = addXMPKV(&input_image_byte, &key_value_json_csz, &output_image_byte);

  t2 = clock();
  float t = float(t2 - t1) / CLOCKS_PER_SEC;

  cszstr_writefile(&output_image_byte, output_file_path, 0);

  cszstr_free(&input_image_byte);
  cszstr_free(&key_value_json_csz);
  cszstr_free(&output_image_byte);
  return r;
}

int main(int argc, char* const argv[]) {
  std::cout << "Start Add XMP for AIGC!\n";

  if (argc != 4) {
    std::cout << "Error input parameters number, need: ./EXE INPUT_FILE_PATH OUTPUT_FILE_PATH XMP_KEY_VALUE_STRING\n";
    return -1;
  } else {
    char* input_file_path = argv[1];
    char* output_file_path = argv[2];
    char* xmp_kv_str = argv[3];

    int ret = main_insert(input_file_path, output_file_path, xmp_kv_str);

    std::cout << "End Add XMP for AIGC!: " << ret << std::endl;

    return ret;
  }
}
