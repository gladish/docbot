#pragma once

#include <curl/curl.h>

#include <cstdint>
#include <list>
#include <optional>
#include <string>

namespace openai {

struct ModelPermission {
  std::string Id;
  std::optional<bool> AllowCreateEngine;
  std::optional<bool> AllowSampling;
  std::optional<bool> AllowLogProbs;
  std::optional<bool> AllowSearchIndices;
  std::optional<bool> AllowView;
  std::optional<bool> AllowFineTuning;
};

struct Model {
  std::string Id;
  std::string Object;
  uint64_t Created;
  std::string OwnedBy;
  ModelPermission Permissions;
};

class Client {
public:
  Client(const std::string &api_key);
  std::list<Model> getModels();

  void documentCode(const std::string &code);
private:
  std::string m_api_key;
};



}