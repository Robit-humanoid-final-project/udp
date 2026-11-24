#include "udp_node.hpp"

#include <iomanip>
#include <limits>
#include <locale>
#include <sstream>

// /TeamData에서 받은 메시지를 순서대로 문자열로 바꿔 상대 NUC에 UDP로 보낸다.
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
             reinterpret_cast<const sockaddr *>(&peer_address_), sizeof(peer_address_)) < 0) {
    RCLCPP_WARN_THROTTLE(get_logger(), *get_clock(), 2000, "UDP send failed");
  }
}
