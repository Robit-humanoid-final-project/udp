#include "udp_node.hpp"
#include "ament_index_cpp/get_package_share_directory.hpp"
#include <vector>
#include <sstream>
#include <locale>
#include <iomanip>
#include <limits>
#include <system_error>

UdpNode::UdpNode() : Node("udp_node") {
    if (socket_.fd < 0) throw std::system_error(errno, std::generic_category(), "socket");
  auto peer_ip = declare_parameter<std::string>("peer_ip");
  auto peer_port = declare_parameter<int>("peer_port");
  auto local_port = declare_parameter<int>("local_port");
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

    received_ = create_publisher<TeamData>("/Team2Data", 10);
    input_ = create_subscription<TeamData>("/TeamData", 10,
      [this](TeamData::ConstSharedPtr msg) { send(*msg); });
    timer_ = create_wall_timer(std::chrono::milliseconds(10), [this] { receive(); });
  }

void UdpNode::send(const TeamData &msg) {
  std::ostringstream out;
  out.imbue(std::locale::classic());
  out << "TEAM1 " << static_cast<unsigned>(msg.robot_id) << ' '
      << static_cast<unsigned>(msg.main_state) << ' ' << static_cast<unsigned>(msg.sub_state)
      << ' ' << static_cast<unsigned>(msg.pass_count)
      << std::setprecision(std::numeric_limits<float>::max_digits10)
      << ' ' << msg.robot_x << ' ' << msg.robot_y << ' ' << msg.robot_theta
      << ' ' << msg.ball_detected << ' ' << msg.ball_x << ' ' << msg.ball_y << ' ' << msg.ball_dist;
  const auto packet = out.str();
  if (sendto(socket_.fd, packet.data(), packet.size(), 0,
             reinterpret_cast<const sockaddr *>(&peer_), sizeof(peer_)) < 0)
    RCLCPP_WARN_THROTTLE(get_logger(), *get_clock(), 2000, "UDP send failed");
}

int main(int argc, char **argv) {
  try {
    const auto config = ament_index_cpp::get_package_share_directory("udp") + "/config/udp.yaml";
    // Load package defaults first so explicit command-line parameters can override them.
    std::vector<const char *> args{argv[0], "--ros-args", "--params-file", config.c_str(), "--"};
    args.insert(args.end(), argv + 1, argv + argc);
    rclcpp::init(static_cast<int>(args.size()), args.data());
    rclcpp::spin(std::make_shared<UdpNode>());
  } catch (const std::exception &e) {
    RCLCPP_ERROR(rclcpp::get_logger("udp_node"), "%s", e.what());
    if (rclcpp::ok()) rclcpp::shutdown();
    return 1;
  }
  rclcpp::shutdown();
  return 0;
}
