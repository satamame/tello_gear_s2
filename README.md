# tello_gear_s2
Gear S2 application to controll Tello.

## Environment

- Tizen IDE 2.4.0_Rev8
- Gear S2
- Ryze Tello

## Feature

- Connect/Disconnect Gear S2 to Tello
- Take off/Land Tello via Gear S2
- Rotate Tello via Gear S2 bezel

## Video

- [Rotating Tello with Gear S2 bezel](https://t.co/UOHdW2yHqh)

## Instruction

1. Import project to Tizen IDE.
    - File/Import -> General/Existing Projects into Workspace
1. Set up certificates.
1. Build and install the app on Gear S2.
1. Turn on Tello.
1. Connect Gear S2 to Tello from Wi-Fi setting.
    - Tello's network setting should be:
        - IP address = 192.168.10.1
        - Port = 8889
1. Go back to the app.
1. Press "Connect".
1. Press "Take off" to fly Tello.
1. You can rotate Tello with Gear S2's bezel.
1. Press "Land" to stop flying Tello.
1. Press "Disconnect" to terminate it.

## Modifying code

- You can modify network information to connect Tello that is hard-coded in btn_connect_clicked_cb function.
- You can modify the rotation angle for each rotary event in _rotary_handler_cb function.

## Limitation

- No error handling for communication with Tello.
