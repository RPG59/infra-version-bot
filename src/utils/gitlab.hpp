#pragma once

#include <userver/clients/http/component.hpp>
#include <userver/components/component.hpp>
#include <userver/crypto/base64.hpp>
#include <userver/formats/yaml.hpp>
#include <userver/server/handlers/http_handler_json_base.hpp>

struct GlProject {
  uint32_t id;
  std::string name;
};

struct GlJob {
  uint32_t id;
  std::string status;
  std::string name;
  std::string ref;
  std::string commitId;
};

using namespace userver;

class Gitlab {
 public:
  std::string origin;
  std::string token;

 public:
  Gitlab(std::string origin, std::string token);
  GlProject getProject(std::string& projectName, std::string& projectNamespace, clients::http::Client& httpClient);
  GlJob getJob(uint32_t projectId, uint32_t jobId, clients::http::Client& httpClient);
  std::optional<std::string> getInfraVersion(uint32_t projectId, std::string& ref, clients::http::Client& httpClient);
  std::optional<std::string> getInfraVersionByJob(uint32_t jobId, std::string& projectName,
                                                  std::string& projectNamespace, clients::http::Client& httpClient);

 private:
  formats::json::Value makeApiRequest(std::string& url, clients::http::Client& httpClient);
};
