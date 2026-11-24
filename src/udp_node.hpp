#pragma once
#include <arpa/inet.h>
#include <unistd.h>
#include "humanoid_interfaces/msg/team_data.hpp"
#include "rclcpp/rclcpp.hpp"

using TeamData = humanoid_interfaces::msg::TeamData;

struct Socket {
  int fd = ::socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
  Socket() = default;
  Socket(const Socket &) = delete;
  Socket &operator=(const Socket &) = delete;

  ~Socket() {
    if (fd >= 0) {
      ::close(fd);
    }
  }
};

class UdpNode : public rclcpp::Node {
public:
  UdpNode();

private:
  void send(const TeamData &msg);
  void receive();

  Socket socket_;
  sockaddr_in peer_address_{};
  rclcpp::Publisher<TeamData>::SharedPtr peer_data_publisher_;
  rclcpp::Subscription<TeamData>::SharedPtr local_data_subscription_;
  rclcpp::TimerBase::SharedPtr receive_timer_;
};
