#include <userver/clients/dns/component.hpp>
#include <userver/clients/http/component.hpp>
#include <userver/components/minimal_server_component_list.hpp>
#include <userver/server/handlers/ping.hpp>
#include <userver/utils/daemon_run.hpp>

#include "infra_version_handler.hpp"

int main(int argc, char* argv[]) {
  auto component_list = userver::components::MinimalServerComponentList()
                            .Append<userver::server::handlers::Ping>()
                            .Append<userver::components::HttpClient>()
                            .Append<userver::clients::dns::Component>();

  component_list.Append<InfraVersionHandler>();

  return userver::utils::DaemonMain(argc, argv, component_list);
}
