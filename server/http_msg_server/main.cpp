/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: http_msg_server.cpp
 Update Time: Thu 15 Jun 2023 00:42:12 CST
 brief:
*/

#include <teamtalk/imcore/common/tools.h>
#include <teamtalk/sbase/version.h>
#include <teamtalk/sbase/global_define.h>
#include <teamtalk/imcore/netlib/core/netlib.h>
#include <teamtalk/imcore/slog/slog.h>

#include "common/server_config/server_config.h"
#include "connection/db_serv_conn.h"
#include "connection/route_serv_conn.h"
#include "common/http/http_conn.h"
#include "common/http/http_query.h"

namespace {

using teamtalk::http_server::common::server_config::ServerConfig;
using teamtalk::http_server::common::server_config::ServerEndpoint;

using teamtalk::http_server::common::http::CHttpConn;
using teamtalk::http_server::common::http::init_http_conn;
using teamtalk::http_server::connection::init_db_serv_conn;
using teamtalk::http_server::connection::init_route_serv_conn;

namespace ttserverinfo = teamtalk::sbase::server_info;
namespace ttnetlib = teamtalk::imcore::netlib;
namespace ttcommon = teamtalk::imcore::common;

ttserverinfo::serv_info_t* to_serv_info_array(const std::vector<ServerEndpoint>& endpoints) {
  if (endpoints.empty()) {
    return nullptr;
  }
  ttserverinfo::serv_info_t* arr = new ttserverinfo::serv_info_t[endpoints.size()];
  for (size_t i = 0; i < endpoints.size(); i++) {
    arr[i].server_ip = endpoints[i].first;
    arr[i].server_port = endpoints[i].second;
  }
  return arr;
}

}  // namespace

void http_callback(void* callback_data, uint8_t msg, uint32_t handle, void* pParam) {
  if (msg == ttnetlib::NETLIB_MSG_CONNECT) {
    CHttpConn* pConn = new CHttpConn();
    pConn->OnConnect(handle);
  } else {
    log_info("!!!error msg: %d ", msg);
  }
}

int main(int argc, char* argv[]) {
  if ((argc == 2) && (strcmp(argv[1], "-v") == 0)) {
    printf("Server Version: HttpMsgServer/%s\n", VERSION);
    printf("Server Build: %s %s\n", __DATE__, __TIME__);
    return 0;
  }

  signal(SIGPIPE, SIG_IGN);
  srand(time(NULL));

  log_info("MsgServer max files can open: %d ", getdtablesize());

  auto& cfg = ServerConfig::Instance();
  if (!cfg.LoadFromFile("http_msg_server.conf")) {
    log_info("config file load failed, exit... ");
    return -1;
  }

  if (!cfg.db_servers().empty()) {
    log_info("DB db_server_count: %zu concurrent_db_conn_cnt: %u expanded_db_conn_cnt: %zu.",
             cfg.db_servers().size(),
             cfg.concurrent_db_conn_cnt(),
             cfg.expanded_db_servers().size());
  }

  if (ttnetlib::netlib_init() == ttnetlib::NETLIB_ERROR) {
    log_error("netlib_init failed, exit... ");
    return -1;
  }

  for (const auto& addr : cfg.listen_addresses()) {
    if (ttnetlib::netlib_listen(addr.c_str(), cfg.listen_port(), http_callback, NULL) == ttnetlib::NETLIB_ERROR) {
      log_error("listen on %s:%d failed, exit... ", addr.c_str(), cfg.listen_port());
      return -1;
    }
  }

  log_info("server start listen on: %s:%d", cfg.listen_addresses().front().c_str(), cfg.listen_port());

  init_http_conn();

  if (!cfg.expanded_db_servers().empty()) {
    uint32_t expanded_cnt = static_cast<uint32_t>(cfg.expanded_db_servers().size());
    ttserverinfo::serv_info_t* db_list = to_serv_info_array(cfg.expanded_db_servers());
    init_db_serv_conn(db_list, expanded_cnt, cfg.concurrent_db_conn_cnt());
  }

  if (!cfg.route_servers().empty()) {
    uint32_t route_cnt = static_cast<uint32_t>(cfg.route_servers().size());
    ttserverinfo::serv_info_t* route_list = to_serv_info_array(cfg.route_servers());
    init_route_serv_conn(route_list, route_cnt);
  }

  log_info("now enter the event loop...");

  ttcommon::write_pid();

  ttnetlib::netlib_eventloop();

  return 0;
}