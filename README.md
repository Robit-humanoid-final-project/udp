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


## YAML파일 준비 

udp.yaml 파일은 .gitignore에 등록되어 있기 때문에 config/udp.yaml은 장치별로 관리한다.

각 로봇은 IP가 다르고 또한 매번 달라지기 때문에 이와 같이 관리한다.

새로 git clone한 경우, 첫 빌드 전에 udp 패키지 안에 config 폴더를 만들고 udp.yaml을 작성한다. 

yaml파일 형식은 다음과 같다.

```yaml
udp_node:
  ros__parameters:
    peer_ip: "127.0.0.1"
    local_port: 5100
    peer_port: 5100
```

한 컴퓨터에서 테스트할 때는 위 설정을 그대로 사용한다.

두 로봇이 통신할 때는 각 로봇의 peer_ip에 상대 로봇의 Wi-Fi IP를 입력한다. peer_port는 상대 로봇의 local_port와 맞춘다.

## 빌드 및 실행 

각 NUC의 `~/colcon_ws/src`에 `udp`와 `TeamData.msg`가 포함된 `humanoid_interfaces` 패키지가 있어야 한다.

```bash
cd ~/colcon_ws &&
source /opt/ros/jazzy/setup.bash &&
colcon build --packages-up-to udp &&
source ~/colcon_ws/install/setup.bash &&
ros2 run udp udp_node
```

## 컴퓨터 한대에서 테스트

config/udp.yaml에서 peer_ip를 127.0.0.1로 설정하고 local_port와 peer_port를 같은 값으로 맞춘다. 

127.0.0.1은 현재 컴퓨터 자신을 가리킨다. 

NUC 두 대에서 테스트할 때는 각 NUC의 peer_ip에 상대 NUC의 Wi-Fi IP를 입력하고, peer_port를 상대의 local_port와 맞춘다.

### 터미널 1

위의 ‘빌드 및 실행’ 명령을 실행하고 노드를 켜 둔다.

```bash
cd ~/colcon_ws &&
source /opt/ros/jazzy/setup.bash &&
colcon build --packages-up-to udp &&
source ~/colcon_ws/install/setup.bash &&
ros2 run udp udp_node
```

### 터미널 2

수신 대기 

```bash
source ~/colcon_ws/install/setup.bash
ros2 topic echo /Team2Data
```
### 터미널 3

테스트 메세지 발행 

```bash
source ~/colcon_ws/install/setup.bash
ros2 topic pub --once /TeamData humanoid_interfaces/msg/TeamData \
  '{robot_id: 1, robot_x: 1.25, ball_detected: true}'
```

터미널 2의 /Team2Data 출력에 TeamData.msg형식으로 표시되면 한 컴퓨터에서 ROS 2 토픽 입력 → UDP 송수신 → ROS 2 토픽 출력까지 확인한 것이다.

## 두 컴퓨터에서 테스트 

### 1. 각 NUC에서 Wi-Fi IP 확인 후 config/udp.yaml 수정

```bash
ip -br -4 addr
```

와이파이가 같은지 확인한다. 

yaml에 작성시에 자신의 IP가 아닌 상대방 눅의 IP를 적어야한다.

포트번호는 같아도 상관없다.

터미널에서 ip -br -4 addr를 치면 다음과 같은형식으로 나온다.

```bash
lo               UNKNOWN        127.0.0.1/8 
wlp0s20f3        UP             172.100.3.33/21 
```

여기서 172.100.3.33이 자신 IP이고 전송하고자 하는 컴퓨터에서 이를 yaml에 입력하면 된다.

첫번째 줄과 '/' 뒤에있는 숫자는 무시한다.


### 2. 터미널 1

각 NUC의 터미널 1에서 노드를 실행한다.

```bash
cd ~/colcon_ws &&
source /opt/ros/jazzy/setup.bash &&
colcon build --packages-up-to udp &&
source ~/colcon_ws/install/setup.bash &&
ros2 run udp udp_node
```

### 3. 터미널 2

각 NUC의 터미널 2에서 수신을 기다린다.

```bash
source ~/colcon_ws/install/setup.bash
ros2 topic echo /Team2Data
```

### 4. 터미널 3

테스트 메세지를 보낸다. 어떤 NUC에 있는 터미널 3에 입력하든 상관없다 

```bash
source ~/colcon_ws/install/setup.bash
ros2 topic pub --once /TeamData humanoid_interfaces/msg/TeamData \
  '{robot_id: 1, robot_x: 1.25, ball_detected: true}'
```

터미널 2의 /Team2Data 출력에 TeamData.msg형식으로 표시되면 두 컴퓨터에서 ROS 2 토픽 입력 → UDP 송수신 → ROS 2 토픽 출력까지 확인한 것이다.