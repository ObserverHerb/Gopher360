#pragma once

#include <iostream>
#include <string>
#include <sstream>
#include <map>
#include <fstream>
#include <vector>
#include "Convert.h"

class ConfigFile
{
private:
  std::map<std::string, std::vector<std::string>> contents;
  std::string fName;

  void removeComment(std::string &line) const;

  bool onlyWhitespace(const std::string &line) const;
  bool validLine(const std::string &line) const;

  void extractKey(std::string &key, size_t const &sepPos, const std::string &line) const;
  void extractValues(std::vector<std::string> &values, size_t const &sepPos, const std::string &line) const;

  void extractContents(const std::string &line);

  void parseLine(const std::string &line, size_t const lineNo);

  void ExtractKeys();

public:
  ConfigFile(const std::string &fName);

  std::vector<std::string> getValuesOfKey(const std::string &key, const std::vector<std::string> &defaultValue = {"0"}) const
  {
    std::map<std::string, std::vector<std::string>>::const_iterator it = contents.find(key);
    if (it == contents.end())
      return defaultValue;

    return it->second;
  };

  void exitWithError(const std::string &error);
};