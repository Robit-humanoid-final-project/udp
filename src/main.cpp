#include "udp_node.hpp"
#include "ament_index_cpp/get_package_share_directory.hpp"

#include <memory>
#include <vector>

// 설정을 읽고 UDP 노드를 실행한다.
int main(int argc, char **argv) {
  try {
    const auto config_path = ament_index_cpp::get_package_share_directory("udp") + "/config/udp.yaml";
    // udp.yaml에서 지정한 값보다 터미널에서 지정된 값을 우선으로 한다.
    std::vector<const char *> args{argv[0], "--ros-args", "--params-file", config_path.c_str(), "--"};
    args.insert(args.end(), argv + 1, argv + argc);
    rclcpp::init(static_cast<int>(args.size()), args.data());
    auto node = std::make_shared<UdpNode>();
    rclcpp::spin(node);
  } catch (const std::exception &e) {
    RCLCPP_ERROR(rclcpp::get_logger("udp_node"), "%s", e.what());
    if (rclcpp::ok()) {
      rclcpp::shutdown();
    }
    return 1;
  }
  rclcpp::shutdown();
  return 0;
}
