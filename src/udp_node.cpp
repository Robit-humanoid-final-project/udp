#include "udp_node.hpp"

#include <cerrno>
#include <chrono>
#include <system_error>

// 상대 NUC의 주소와 내 수신 포트를 설정하고, ROS 2 토픽과 UDP 수신 타이머를 준비한다.
UdpNode::UdpNode() : Node("udp_node") {
  if (socket_.fd < 0) {
    throw std::system_error(errno, std::generic_category(), "socket");
  }

  const auto peer_ip = declare_parameter<std::string>("peer_ip");
  const auto peer_port = declare_parameter<int>("peer_port");
  const auto local_port = declare_parameter<int>("local_port");
  if (peer_port < 1 || peer_port > 65535 || local_port < 1 || local_port > 65535) {
    throw std::invalid_argument("ports must be within 1..65535");
  }
  peer_address_.sin_family = AF_INET;
  peer_address_.sin_port = htons(peer_port);
  if (inet_pton(AF_INET, peer_ip.c_str(), &peer_address_.sin_addr) != 1) {
    throw std::invalid_argument("peer_ip requires an IPv4 address");
  }

  sockaddr_in local{};
  local.sin_family = AF_INET;
  local.sin_port = htons(local_port);
  if (bind(socket_.fd, reinterpret_cast<sockaddr *>(&local), sizeof(local)) < 0) {
    throw std::system_error(errno, std::generic_category(), "bind");
  }

  peer_data_publisher_ = create_publisher<TeamData>("/Team2Data", 10);
  local_data_subscription_ = create_subscription<TeamData>("/TeamData", 10,
    [this](TeamData::ConstSharedPtr msg) { send(*msg); });
  receive_timer_ = create_wall_timer(std::chrono::milliseconds(10), [this] { receive(); });
}
