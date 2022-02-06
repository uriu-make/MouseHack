#include "shortcut.h"

int main() {
  while (true) {
    std::string str, device, touchpad, buf;
    std::size_t search = std::string::npos;
    //デバイスの捜索
    device = SearchDevice(DEVICE_NAME);
    touchpad = SearchDevice(TOUCHPAD);

    int mousefd = open(device.c_str(), O_RDWR);  //見つけたデバイスを開く
    int touchpadfd = open(touchpad.c_str(), O_RDWR);
    int uinputfd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    ioctl(mousefd, EVIOCGRAB, 1);  //マウスを無効化
    ioctl(touchpadfd, EVIOCGRAB, 1);

    ioctl(uinputfd, UI_SET_EVBIT, EV_REL);  //使用するマウスの機能
    ioctl(uinputfd, UI_SET_RELBIT, REL_X);
    ioctl(uinputfd, UI_SET_RELBIT, REL_Y);
    ioctl(uinputfd, UI_SET_RELBIT, REL_WHEEL);
    ioctl(uinputfd, UI_SET_EVBIT, EV_KEY);  //使用するキーとボタン
    ioctl(uinputfd, UI_SET_KEYBIT, BTN_LEFT);
    ioctl(uinputfd, UI_SET_KEYBIT, BTN_RIGHT);
    ioctl(uinputfd, UI_SET_KEYBIT, BTN_MIDDLE);
    ioctl(uinputfd, UI_SET_KEYBIT, KEY_BACK);
    ioctl(uinputfd, UI_SET_KEYBIT, KEY_FORWARD);
    ioctl(uinputfd, UI_SET_KEYBIT, KEY_TAB);
    ioctl(uinputfd, UI_SET_KEYBIT, KEY_LEFTALT);
    ioctl(uinputfd, UI_SET_KEYBIT, KEY_LEFTCTRL);
    ioctl(uinputfd, UI_SET_KEYBIT, KEY_D);
    ioctl(uinputfd, UI_SET_KEYBIT, KEY_0);
    create_uinput_device(uinputfd);  //マウスとして登録

    struct event_data data[8];
    short int result = 0;
    struct timeval time;
    gettimeofday(&time, NULL);
    struct input_event event, output;
    struct timetable t;

    while (result == 0) {
      result = readevent(mousefd, &event, &t);
      if (result == 0) {
        if (event.type == EV_KEY) {  //ボタンの入力ならばボタンの種類ごとに処理
          switch (event.code) {
            case BTN_LEFT:  //各ボタンの状態とイベント以前の状態を記録しイベントが発生してからの経過時間を初期化
              data[LEFT].button_old = data[LEFT].button;
              data[LEFT].button = event.value;
              data[LEFT].timer = 0;
              break;
            case BTN_RIGHT:
              data[RIGHT].button_old = data[RIGHT].button;
              data[RIGHT].button = event.value;
              data[RIGHT].timer = 0;
              break;
            case BTN_MIDDLE:
              data[MIDDLE].button_old = data[MIDDLE].button;
              data[MIDDLE].button = event.value;
              data[MIDDLE].timer = 0;
              break;
            case BTN_SIDE:
              data[SIDE].button_old = data[SIDE].button;
              data[SIDE].button = event.value;
              data[SIDE].timer = 0;
              break;
            case BTN_EXTRA:
              data[EXTRA].button_old = data[EXTRA].button;
              data[EXTRA].button = event.value;
              data[EXTRA].timer = 0;
              break;
            case BTN_FORWARD:
              data[FORWARD].button_old = data[FORWARD].button;
              data[FORWARD].button = event.value;
              data[FORWARD].timer = 0;
              break;
            case BTN_BACK:
              data[BACK].button_old = data[BACK].button;
              data[BACK].button = event.value;
              data[BACK].timer = 0;
              break;
            case BTN_TASK:
              data[TASK].button_old = data[TASK].button;
              data[TASK].button = event.value;
              data[TASK].timer = 0;
              break;
            default:  //以上に当てはまらなければそのままイベントを送信
              write(uinputfd, &event, sizeof(struct input_event));
              break;
          }
        } else {  //その他のイベントならばそのまま送信
          write(uinputfd, &event, sizeof(struct input_event));
        }
        left_func(mousefd, uinputfd, &data[LEFT], &event, &t);
        right_func(mousefd, uinputfd, &data[RIGHT], &event, &t);
        result = middle_func(mousefd, uinputfd, &data[MIDDLE], &event, &t);
        side_func(mousefd, uinputfd, &data[SIDE], &event, &t);
        extra_func(mousefd, uinputfd, &data[EXTRA], &event, &t);
        forward_func(mousefd, uinputfd, &data[FORWARD], &event, &t);
        back_func(mousefd, uinputfd, &data[BACK], &event, &t);
        task_func(mousefd, uinputfd, &data[TASK], &event, &t);
      }
      if (std::filesystem::exists(device)) {
        result = 0;
      } else {
        result = -1;
      }
    }
    ioctl(touchpadfd, EVIOCGRAB, 0);
    ioctl(mousefd, EVIOCGRAB, 0);
    close(mousefd);
    ioctl(uinputfd, UI_DEV_DESTROY);
    close(uinputfd);
  }
  return 0;
}