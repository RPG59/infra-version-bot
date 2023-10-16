#pragma once

#include <userver/clients/http/component.hpp>
#include <userver/components/component.hpp>
#include <userver/crypto/base64.hpp>
#include <userver/engine/wait_all_checked.hpp>
#include <userver/engine/wait_any.hpp>
#include <userver/formats/yaml.hpp>
#include <userver/http/parser/http_request_parse_args.hpp>
#include <userver/server/handlers/exceptions.hpp>
#include <userver/server/handlers/http_handler_json_base.hpp>
#include <userver/utils/async.hpp>

#include "utils/gitlab.hpp"

class InfraVersionHandler final : public server::handlers::HttpHandlerJsonBase {
 private:
  Gitlab* gitlab;

 public:
  uint32_t infraProjectId = 25854;
  clients::http::Client& http_client;

  using KeyValues = std::unordered_map<std::string, formats::json::Value>;
  static constexpr std::string_view kName = "handler-infra-version";

  InfraVersionHandler(const components::ComponentConfig& config, const components::ComponentContext& context);

  ~InfraVersionHandler();

  formats::json::Value HandleRequestJsonThrow(const server::http::HttpRequest&, const formats::json::Value&,
                                              server::request::RequestContext&) const override;

  const std::string getInfraVersion(uint32_t projectId, std::string ref, std::string glToken) const;
};
