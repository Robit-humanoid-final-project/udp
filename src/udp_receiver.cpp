#include "udp_node.hpp"
#include <sstream>
#include <locale>
#include <cerrno>

void UdpNode::receive() {
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
      TeamData msg;
      unsigned id, main_state, sub_state, passes;
      if (!(in >> tag >> id >> main_state >> sub_state >> passes
               >> msg.robot_x >> msg.robot_y >> msg.robot_theta >> msg.ball_detected
               >> msg.ball_x >> msg.ball_y >> msg.ball_dist) ||
          tag != "TEAM1" || (in >> extra) ||
          id > 255 || main_state > 255 || sub_state > 255 || passes > 255)
        continue;
      msg.robot_id = id;
      msg.main_state = main_state;
      msg.sub_state = sub_state;
      msg.pass_count = passes;
      received_->publish(msg);
    }
  }
