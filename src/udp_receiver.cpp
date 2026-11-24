#include "udp_node.hpp"
#include <sstream>
#include <locale>
#include <cerrno>

// 상대 NUC에서 온 UDP 데이터의 형식을 확인하고, TeamData 메시지로 변환해 /Team2Data에 발행한다.
void UdpNode::receive() {
  constexpr int max_packets_per_poll = 32;
  for (int i = 0; i < max_packets_per_poll; ++i) {
    char buffer[256];
    sockaddr_in sender{};
    socklen_t length = sizeof(sender);
    auto bytes_received = recvfrom(socket_.fd, buffer, sizeof(buffer), MSG_TRUNC,
                      reinterpret_cast<sockaddr *>(&sender), &length);
    if (bytes_received < 0) {
      if (errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR) {
        RCLCPP_WARN_THROTTLE(get_logger(), *get_clock(), 2000, "UDP receive failed");
      }
      return;
    }
    if (bytes_received > static_cast<ssize_t>(sizeof(buffer)) ||
        sender.sin_addr.s_addr != peer_address_.sin_addr.s_addr || sender.sin_port != peer_address_.sin_port) {
      continue;
    }
    std::istringstream in(std::string(buffer, bytes_received));
    in.imbue(std::locale::classic());
    std::string tag, extra;
    TeamData msg;
    unsigned id, main_state, sub_state, passes;
    if (!(in >> tag >> id >> main_state >> sub_state >> passes
             >> msg.robot_x >> msg.robot_y >> msg.robot_theta >> msg.ball_detected
             >> msg.ball_x >> msg.ball_y >> msg.ball_dist) ||
        tag != "TEAM1" || (in >> extra) ||
        id > 255 || main_state > 255 || sub_state > 255 || passes > 255) {
      continue;
    }
    msg.robot_id = id;
    msg.main_state = main_state;
    msg.sub_state = sub_state;
    msg.pass_count = passes;
    peer_data_publisher_->publish(msg);
  }
}
