#include "infra_version_handler.hpp"

InfraVersionHandler::InfraVersionHandler(const components::ComponentConfig& config,
                                         const components::ComponentContext& context)
    : server::handlers::HttpHandlerJsonBase(config, context),
      httpClient(context.FindComponent<components::HttpClient>().GetHttpClient()) {
  auto glToken = std::getenv("GL_TOKEN");
  auto glOrigin = std::getenv("GL_ORIGIN");

  if (glToken == nullptr) {
    throw std::runtime_error("Invalid GL_TOKEN");
  }

  if (glOrigin == nullptr) {
    throw std::runtime_error("Invalid GL_ORIGIN");
  }

  gitlab = new Gitlab(glOrigin, glToken);
}

InfraVersionHandler::~InfraVersionHandler() { delete gitlab; }

formats::json::Value InfraVersionHandler::HandleRequestJsonThrow(const server::http::HttpRequest&,
                                                                 const formats::json::Value& request,
                                                                 server::request::RequestContext&) const {
  auto jobId = request["jobId"].As<uint32_t>(0);
  auto projectName = request["projectName"].As<std::string>("");
  auto projectNamespace = request["projectNamespace"].As<std::string>("");

  if (jobId == 0 || projectName.empty() || projectNamespace.empty()) {
    throw server::handlers::ExceptionWithCode<server::handlers::HandlerErrorCode::kClientError>();
  }

  std::string infraMasterBranchName = "master";
  auto masterInfraVersionTask =
      utils::Async("masterInfraVersionTask", [this, &infraMasterBranchName = infraMasterBranchName] {
        return gitlab->getInfraVersion(infraProjectId, infraMasterBranchName, httpClient);
      });

  auto currentInfraVersionTask =
      utils::Async("currentInfraVersionTask", [this, jobId = jobId, &projectName = projectName, &projectNamespace = projectNamespace] {
        return gitlab->getInfraVersionByJob(jobId, projectName, projectNamespace, httpClient);
      });

  engine::WaitAllChecked(masterInfraVersionTask);

  auto currentInfraVersion = currentInfraVersionTask.Get();
  auto masterInfraVersion = masterInfraVersionTask.Get();

  if (!masterInfraVersion.has_value() || !currentInfraVersion.has_value()) {
    throw server::handlers::ExceptionWithCode<server::handlers::HandlerErrorCode::kBadGateway>();
  }

  formats::json::ValueBuilder res;

  res["current"] = currentInfraVersion.value();
  res["master"] = masterInfraVersion.value();

  return res.ExtractValue();
}
