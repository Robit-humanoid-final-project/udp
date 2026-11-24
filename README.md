## TaskPlanner와 연결하는 토픽

| 토픽 | 발행 → 구독 | 내용 |
|---|---|---|
| `/TeamData` | TaskPlanner → udp_node | 내 로봇의 정보 |
| `/Team2Data` | udp_node → TaskPlanner | 상대 로봇에서 수신한 정보 |

두 토픽 모두 `humanoid_interfaces/msg/TeamData` 형식을 이용한다.

 송신은 `src/udp_sender.cpp`, 수신은 `src/udp_receiver.cpp`에서 처리하며 두 파일은 하나의 실행 파일로 빌드한다.

## 빌드 방법

각 NUC의 `~/colcon_ws/src`에 `udp`와 `TeamData.msg`가 포함된 `humanoid_interfaces` 패키지가 있어야 한다.

```bash
cd ~/colcon_ws
source /opt/ros/jazzy/setup.bash
colcon build --packages-up-to udp
source install/setup.bash
```

## 실행 방법

```bash
source ~/colcon_ws/install/setup.bash
ros2 run udp udp_node
```

## yaml

yaml파일에서 ip와 port를 관리할 수 있도록 하였다.

```yaml
udp_node:
  ros__parameters:
    peer_ip: "127.0.0.1"
    local_port: 5100
    peer_port: 5100
```


