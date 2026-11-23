# UDP 로봇 위치 통신

두 로봇이 각자의 전역 위치와 방향 `(x, y, theta)`을 UDP로 주고받는 ROS 2 Jazzy 패키지.
`Localization`이 `/world_model`을 발행할 때마다 `robot_pose`를 상대 로봇으로 보내고, 상대에게서 받은 좌표는 `/udp/peer_pose`로 발행한다.

## ROS 2 토픽

| 구분 | 토픽 | 메시지 타입 | 데이터 |
|---|---|---|---|
| 구독 | `/world_model` | `humanoid_interfaces/msg/WorldModel` | 내 로봇의 `robot_pose` |
| 발행 | `/udp/peer_pose` | `geometry_msgs/msg/Pose2D` | 상대 로봇의 `x`, `y`, `theta` |

현재 버전은 `is_localized` 값과 관계없이 `robot_pose`를 전송한다.

## 빌드

`udp`와 `humanoid_interfaces`를 `~/colcon_ws/src`에 둔 뒤 실행한다.

```bash
cd ~/colcon_ws
source /opt/ros/jazzy/setup.bash
colcon build --packages-up-to udp
source install/setup.bash
```

## NUC 두 대에서 실행

두 NUC를 같은 네트워크에 연결하고 각각 패키지를 빌드한다. 아래 IP는 예시이므로 `peer_ip`를 상대 NUC의 실제 IPv4 주소로 바꾼다.

| 장치 | 예시 IP | 내 수신 포트 (`local_port`) | 상대 수신 포트 (`peer_port`) |
|---|---|---:|---:|
| NUC 1 | `192.168.0.101` | 5000 | 5001 |
| NUC 2 | `192.168.0.102` | 5001 | 5000 |

**NUC 1**

```bash
ros2 run udp udp_node --ros-args \
  -p local_port:=5000 -p peer_ip:=192.168.0.102 -p peer_port:=5001
```

**NUC 2**

```bash
ros2 run udp udp_node --ros-args \
  -p local_port:=5001 -p peer_ip:=192.168.0.101 -p peer_port:=5000
```

각 NUC의 `/world_model`에는 해당 로봇의 위치만 들어와야 한다. 두 NUC가 같은 네트워크에서 동일한 토픽 이름을 사용한다면, NUC별로 `ROS_DOMAIN_ID`를 다르게 설정하고 같은 NUC의 모든 ROS 2 터미널에는 같은 값을 적용한다.

## NUC 한 대에서 송수신 테스트

한 NUC에서 터미널 세 개를 열고, 각 터미널에서 먼저 `source ~/colcon_ws/install/setup.bash`를 실행한다.

**터미널 1**에서 목적지를 자기 자신으로 지정하고 송신·수신 포트를 맞춘다.

```bash
ros2 run udp udp_node --ros-args \
  -p local_port:=5000 -p peer_ip:=127.0.0.1 -p peer_port:=5000
```

**터미널 2**에서 수신 좌표를 기다린다.

```bash
ros2 topic echo /udp/peer_pose
```

**터미널 3**에서 테스트 좌표를 한 번 발행한다.

```bash
ros2 topic pub --once /world_model humanoid_interfaces/msg/WorldModel \
  '{robot_pose: {x: 1.2, y: 0.7, theta: 0.3}}'
```

터미널 2에 `x: 1.2`, `y: 0.7`, `theta: 0.3`이 출력되면 UDP 송수신을 확인할 수 있다.