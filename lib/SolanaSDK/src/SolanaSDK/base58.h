#ifndef BASE58_H
#define BASE58_H

#include <vector>
#include <string>

class Base58
{
public:
  Base58();
  static void printArray(const std::vector<unsigned char> &arr);
  static std::string encode(const std::vector<uint8_t> &input);
  static std::vector<uint8_t> decode(const std::string &addr);
  static std::string trimEncode(const std::vector<uint8_t> &input);
  static std::vector<uint8_t> trimDecode(const std::string &addr);

private:
  static const std::string ALPHABET;
};

#endif // BASE58_H
