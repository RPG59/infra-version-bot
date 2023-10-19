#include "gitlab.hpp"

GlJob Parse(const formats::json::Value& json, formats::parse::To<GlJob>) {
  return GlJob{json["id"].As<uint32_t>(0), json["name"].As<std::string>(""), json["status"].As<std::string>(""),
               json["ref"].As<std::string>(""), json["commit"]["id"].As<std::string>("")};
}

formats::json::Value Serialize(const GlProject& data, formats::serialize::To<formats::json::Value>) {
  formats::json::ValueBuilder builder;
  builder["id"] = data.id;
  builder["name"] = data.name;

  return builder.ExtractValue();
}

GlProject Parse(const formats::json::Value& json, formats::parse::To<GlProject>) {
  return GlProject{json["id"].As<uint32_t>(0), json["name"].As<std::string>("")};
}

Gitlab::Gitlab(std::string origin, std::string token) : origin(origin), token(token) {}

GlProject Gitlab::getProject(std::string& projectName, std::string& projectNamespace,
                             clients::http::Client& httpClient) {
  auto url = fmt::format("{}/api/v4/projects/{}%2F{}", origin, projectNamespace, projectName);

  return makeApiRequest(url, httpClient).As<GlProject>();
}

GlJob Gitlab::getJob(uint32_t projectId, uint32_t jobId, clients::http::Client& httpClient) {
  auto url = fmt::format("{}/api/v4/projects/{}/jobs/{}", origin, projectId, jobId);

  return makeApiRequest(url, httpClient).As<GlJob>();
}

std::optional<std::string> Gitlab::getInfraVersion(uint32_t projectId, std::string& ref,
                                                   clients::http::Client& httpClient) {
  auto url = fmt::format("{}/api/v4/projects/{}/repository/files/.gitlab-ci.yml?ref={}", origin, projectId, ref);
  auto jsonData = makeApiRequest(url, httpClient);
  auto b64Data = jsonData["content"].As<std::string>("");

  if (b64Data.empty()) {
    return std::nullopt;
  }

  auto yamlDataString = crypto::base64::Base64Decode(b64Data);
  auto yamlData = formats::yaml::FromString(yamlDataString);
  auto infraVersion = yamlData["variables"]["SD_CI_VERSION"].As<std::string>("");

  if (infraVersion.empty()) {
    return std::nullopt;
  }

  return infraVersion;
}

formats::json::Value Gitlab::makeApiRequest(std::string& url, clients::http::Client& httpClient) {
  http::headers::HeaderMap headersMap({{"PRIVATE-TOKEN", token}});

  auto glResponse = httpClient.CreateRequest()
                        .get(url)
                        .retry(0)
                        .timeout(std::chrono::milliseconds{10000})
                        .headers(headersMap)
                        .perform();

  glResponse->raise_for_status();

  return formats::json::FromString(glResponse->body_view());
}

std::optional<std::string> Gitlab::getInfraVersionByJob(uint32_t jobId, std::string& projectName,
                                                        std::string& projectNamespace,
                                                        clients::http::Client& httpClient) {
  auto currentProject = getProject(projectName, projectNamespace, httpClient);
  auto currentJob = getJob(currentProject.id, jobId, httpClient);

  return getInfraVersion(currentProject.id, currentJob.commitId, httpClient);
}
