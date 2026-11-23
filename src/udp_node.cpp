#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cerrno>
#include <chrono>
#include <cmath>
#include <iomanip>
#include <limits>
#include <locale>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <system_error>

#include "geometry_msgs/msg/pose2_d.hpp"
#include "humanoid_interfaces/msg/world_model.hpp"
#include "rclcpp/rclcpp.hpp"

using Pose = geometry_msgs::msg::Pose2D;
using WorldModel = humanoid_interfaces::msg::WorldModel;

struct Socket {
  int fd = ::socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
  ~Socket() { if (fd >= 0) ::close(fd); }
};

bool finite_pose(const Pose &p) {
  return std::isfinite(p.x) && std::isfinite(p.y) && std::isfinite(p.theta);
}

class UdpNode : public rclcpp::Node {
public:
  UdpNode() : Node("udp_node") {
    if (socket_.fd < 0) throw std::system_error(errno, std::generic_category(), "socket");
    auto peer_ip = declare_parameter<std::string>("peer_ip", "127.0.0.1");
    auto peer_port = declare_parameter<int>("peer_port", 5001);
    auto local_port = declare_parameter<int>("local_port", 5000);
    if (peer_port < 1 || peer_port > 65535 || local_port < 1 || local_port > 65535)
      throw std::invalid_argument("ports must be within 1..65535");
    peer_.sin_family = AF_INET;
    peer_.sin_port = htons(peer_port);
    if (inet_pton(AF_INET, peer_ip.c_str(), &peer_.sin_addr) != 1)
      throw std::invalid_argument("peer_ip requires an IPv4 address");

    sockaddr_in local{};
    local.sin_family = AF_INET;
    local.sin_port = htons(local_port);
    if (bind(socket_.fd, reinterpret_cast<sockaddr *>(&local), sizeof(local)) < 0)
      throw std::system_error(errno, std::generic_category(), "bind");

    received_ = create_publisher<Pose>("udp/peer_pose", 10);
    input_ = create_subscription<WorldModel>("world_model", 10,
      [this](WorldModel::ConstSharedPtr msg) { send_pose(msg->robot_pose); });
    timer_ = create_wall_timer(std::chrono::milliseconds(10), [this] { receive(); });
  }

private:
  void send_pose(const Pose &pose) {
    if (!finite_pose(pose)) return;
    std::ostringstream out;
    out.imbue(std::locale::classic());
    out << "POSE1 " << std::setprecision(std::numeric_limits<double>::max_digits10)
        << pose.x << ' ' << pose.y << ' ' << pose.theta;
    auto packet = out.str();
    if (sendto(socket_.fd, packet.data(), packet.size(), 0,
               reinterpret_cast<const sockaddr *>(&peer_), sizeof(peer_)) < 0)
      RCLCPP_WARN_THROTTLE(get_logger(), *get_clock(), 2000, "UDP send failed");
  }

  void receive() {
    for (int i = 0; i < 32; ++i) {
      char buffer[256];
      sockaddr_in sender{};
      socklen_t length = sizeof(sender);
      auto n = recvfrom(socket_.fd, buffer, sizeof(buffer), MSG_TRUNC,
                        reinterpret_cast<sockaddr *>(&sender), &length);
      if (n < 0) {
        if (errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR)
          RCLCPP_WARN_THROTTLE(get_logger(), *get_clock(), 2000, "UDP receive failed");
        return;
      }
      if (n > static_cast<ssize_t>(sizeof(buffer)) ||
          sender.sin_addr.s_addr != peer_.sin_addr.s_addr || sender.sin_port != peer_.sin_port)
        continue;
      std::istringstream in(std::string(buffer, n));
      in.imbue(std::locale::classic());
      std::string tag, extra;
      Pose p;
      if ((in >> tag >> p.x >> p.y >> p.theta) && tag == "POSE1" &&
          !(in >> extra) && finite_pose(p))
        received_->publish(p);
    }
  }

  Socket socket_;
  sockaddr_in peer_{};
  rclcpp::Publisher<Pose>::SharedPtr received_;
  rclcpp::Subscription<WorldModel>::SharedPtr input_;
  rclcpp::TimerBase::SharedPtr timer_;
};

int main(int argc, char **argv) {
  rclcpp::init(argc, argv);
  try {
    rclcpp::spin(std::make_shared<UdpNode>());
  } catch (const std::exception &e) {
    RCLCPP_ERROR(rclcpp::get_logger("udp_node"), "%s", e.what());
    rclcpp::shutdown();
    return 1;
  }
  rclcpp::shutdown();
  return 0;
}
