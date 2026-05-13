#pragma once

#include <map>
#include <string>

class TextResources
{
  public:
    static void loadAll();
    static std::string get(const std::string &key);
    static std::string format(const std::string &key, const std::map<std::string, std::string> &replacements);

  private:
    static std::map<std::string, std::string> strings;

    static void loadStrings();
};
