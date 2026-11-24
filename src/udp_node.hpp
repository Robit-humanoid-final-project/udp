#pragma once
#include <arpa/inet.h>
#include <unistd.h>
#include "humanoid_interfaces/msg/team_data.hpp"
#include "rclcpp/rclcpp.hpp"

using TeamData = humanoid_interfaces::msg::TeamData;

struct Socket {
  int fd = ::socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
  ~Socket() { if (fd >= 0) ::close(fd); }
};

class UdpNode : public rclcpp::Node {
public:
  UdpNode();

private:
  void send(const TeamData &msg);
  void receive();

  Socket socket_;
  sockaddr_in peer_{};
  rclcpp::Publisher<TeamData>::SharedPtr received_;
  rclcpp::Subscription<TeamData>::SharedPtr input_;
  rclcpp::TimerBase::SharedPtr timer_;
};

