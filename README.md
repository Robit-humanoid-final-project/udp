## TaskPlanner와 연결하는 토픽

| 토픽 | 발행 → 구독 | 내용 |
|---|---|---|
| `/TeamData` | TaskPlanner → udp_node | 내 로봇의 정보 |
| `/Team2Data` | udp_node → TaskPlanner | 상대 로봇에서 수신한 정보 |

두 토픽 모두 `humanoid_interfaces/msg/TeamData` 형식을 이용한다.

## 파일별 역할

| 파일 | 역할 |
|---|---|
| `src/main.cpp` | YAML 경로를 지정하고 ROS 2와 노드를 실행 |
| `src/udp_node.hpp` | 노드가 사용하는 소켓, 함수, 토픽을 선언 |
| `src/udp_node.cpp` | 포트를 열고 송신 콜백과 수신 타이머를 연결|
| `src/udp_sender.cpp` | 내 팀 데이터를 문자열로 변환해 상대에게 송신|
| `src/udp_receiver.cpp` | 상대 패킷을 확인하고 팀 메시지로 복원해 수신 |

## 빌드 및 실행 

각 NUC의 `~/colcon_ws/src`에 `udp`와 `TeamData.msg`가 포함된 `humanoid_interfaces` 패키지가 있어야 한다.

```bash
cd ~/colcon_ws &&
source /opt/ros/jazzy/setup.bash &&
colcon build --packages-select udp &&
source ~/colcon_ws/install/setup.bash &&
ros2 run udp udp_node
```

## yaml

yaml파일에서 ip와 port를 관리할 수 있도록 하였다.


```yaml
# 예시
udp_node:
  ros__parameters:
    peer_ip: "127.0.0.1"
    local_port: 5100
    peer_port: 5100
```


