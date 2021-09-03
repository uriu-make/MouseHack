#include "shortcut.h"

void create_uinput_device(int fd) {
  struct uinput_user_dev uidev;
  memset(&uidev, 0, sizeof(uidev));

  snprintf(uidev.name, UINPUT_MAX_NAME_SIZE, "CastamMouse");
  uidev.id.bustype = BUS_USB;
  uidev.id.vendor = 0xAAAA;
  uidev.id.product = 0xBBBB;
  uidev.id.version = 1;

  if (write(fd, &uidev, sizeof(uidev)) < 0)
    die("create_uinput_device: write");

  if (ioctl(fd, UI_DEV_CREATE) < 0)
    die("create_uinput_device: ioctl");
}
//イベントの受信
short int readevent(int fd, struct input_event *event, struct timetable *t) {
  struct timeval time;
  struct pollfd fds;
  fds.fd = fd;
  nfds_t nfds = 1;
  short result = 0;
  fds.events = 0;
  poll(&fds, nfds, 1);  //マウスのイベントが読み取り可能かタイムアウト付きで確認
  result = fds.revents;
  if (result == 0) {
    fds.events = POLLIN;
    poll(&fds, nfds, 1);
    if (fds.revents == POLLIN)  //読み取り可能ならば読み込む
      read(fd, event, sizeof(input_event));
    gettimeofday(&time, NULL);  //1ループの時間計測
    t->current_time = time.tv_sec * 1000000 + time.tv_usec;
    t->runtime = t->current_time - t->old_time;
    t->old_time = t->current_time;
  }
  return result;
}
//イベントの送信
void sendevent(int output, int type, int code, int value) {
  struct input_event event;
  event.type = type;
  event.code = code;
  event.value = value;
  gettimeofday(&event.time, NULL);
  write(output, &event, sizeof(struct input_event));
}

short int left_func(int mousefd, int uinputfd, struct event_data *data, struct input_event *event, struct timetable *t) {
  short int result = 0;
  sendevent(uinputfd, EV_KEY, BTN_LEFT, data->button);
  // sendevent(uinputfd, EV_SYN, SYN_REPORT, 0);
  return result;
}

short int right_func(int mousefd, int uinputfd, struct event_data *data, struct input_event *event, struct timetable *t) {
  short int result = 0;
  write(uinputfd, event, sizeof(struct input_event));
  sendevent(uinputfd, EV_KEY, BTN_RIGHT, data->button);
  // sendevent(uinputfd, EV_SYN, SYN_REPORT, 0);
  return result;
}

short int middle_func(int mousefd, int uinputfd, struct event_data *data, struct input_event *event, struct timetable *t) {
  short int result = 0;
  if (data->button == 1 && data->button_old == 0) {  //ミドルクリックでジェスチャー
    int x = 0, y = 0, wheel = 0;
    while (data->button == 1 && result == 0) {
      result = readevent(mousefd, event, t);
      if (event->type == EV_REL) {
        switch (event->code) {
          case REL_X:
            x += event->value;
            break;
          case REL_Y:
            y += event->value;
            break;
        }
      } else if (event->type == EV_KEY && event->code == BTN_MIDDLE) {
        data->button_old = data->button;
        data->button = event->value;
      }
      if (y <= -75 && abs(x) < 50) {  //上下方向に動かすとCtrl+マウスホイール上下に置き換え
        wheel = 1;
        y = 0;
        x = 0;
        sendevent(uinputfd, EV_KEY, KEY_LEFTCTRL, 1);
        sendevent(uinputfd, EV_REL, REL_WHEEL, wheel);
        sendevent(uinputfd, EV_SYN, SYN_REPORT, 0);
      } else if (y >= 75 && abs(x) < 50) {
        wheel = -1;
        y = 0;
        x = 0;
        sendevent(uinputfd, EV_KEY, KEY_LEFTCTRL, 1);
        sendevent(uinputfd, EV_REL, REL_WHEEL, wheel);
        sendevent(uinputfd, EV_SYN, SYN_REPORT, 0);
      } else if (x >= 50 && abs(y) < 50) {  //左右方向の移動があれば進む/戻るに置換
        y = 0;
        x = 0;
        sendevent(uinputfd, EV_KEY, KEY_FORWARD, 1);
        sendevent(uinputfd, EV_SYN, SYN_REPORT, 0);
      } else if (x <= -50 && abs(y) < 50) {
        y = 0;
        x = 0;
        sendevent(uinputfd, EV_KEY, KEY_BACK, 1);
        sendevent(uinputfd, EV_SYN, SYN_REPORT, 0);
      }
    }
    if (x == 0 && y == 0) {  //ミドルクリック終了までに操作がなければ拡大率を初期化(Ctrl+0)
      sendevent(uinputfd, EV_KEY, KEY_LEFTCTRL, 1);
      sendevent(uinputfd, EV_KEY, KEY_0, 1);
      sendevent(uinputfd, EV_SYN, SYN_REPORT, 0);
    }
  } else if (data->button == 0 && data->button_old == 1) {
    sendevent(uinputfd, EV_KEY, KEY_LEFTCTRL, 0);
    sendevent(uinputfd, EV_KEY, KEY_FORWARD, 0);
    sendevent(uinputfd, EV_KEY, KEY_BACK, 0);
    sendevent(uinputfd, EV_KEY, KEY_0, 0);
    sendevent(uinputfd, EV_SYN, SYN_REPORT, 0);
    data->button_old = 0;
  }
  return result;
}

short int side_func(int mousefd, int uinputfd, struct event_data *data, struct input_event *event, struct timetable *t) {
  short int result = 0;
  if (data->button == 1 && data->button_old == 0) {  //SIDEボタンを押したならばCtrl+Alt+Dを送信
    sendevent(uinputfd, EV_KEY, KEY_LEFTCTRL, 1);
    sendevent(uinputfd, EV_KEY, KEY_LEFTALT, 1);
    sendevent(uinputfd, EV_KEY, KEY_D, 1);
    sendevent(uinputfd, EV_SYN, SYN_REPORT, 0);
  } else if (data->button == 0 && data->button_old == 1) {
    sendevent(uinputfd, EV_KEY, KEY_LEFTCTRL, 0);
    sendevent(uinputfd, EV_KEY, KEY_LEFTALT, 0);
    sendevent(uinputfd, EV_KEY, KEY_D, 0);
    sendevent(uinputfd, EV_SYN, SYN_REPORT, 0);
    data->button_old = 0;
  }
  return result;
}

short int extra_func(int mousefd, int uinputfd, struct event_data *data, struct input_event *event, struct timetable *t) {
  short int result = 0;
  if (data->button == 1) {  //EXTRAボタンならばAilt+Tabを送信
    sendevent(uinputfd, EV_KEY, KEY_LEFTALT, 1);
    sendevent(uinputfd, EV_KEY, KEY_TAB, 1);
    sendevent(uinputfd, EV_SYN, SYN_REPORT, 0);
    data->timer = 0;
  } else if (data->button == 0 && data->button_old == 1 && data->timer < 1000000) {  //EXTRAボタンを離しても1秒以内ならばAltキーを送信
    sendevent(uinputfd, EV_KEY, KEY_LEFTALT, 1);
    sendevent(uinputfd, EV_KEY, KEY_TAB, 0);
    sendevent(uinputfd, EV_SYN, SYN_REPORT, 0);
    data->timer += t->runtime;
    data->button_old = 1;
  } else {  //キー入力終了
    sendevent(uinputfd, EV_KEY, KEY_LEFTALT, 0);
    sendevent(uinputfd, EV_KEY, KEY_TAB, 0);
    sendevent(uinputfd, EV_SYN, SYN_REPORT, 0);
    data->button_old = 0;
    data->timer = 0;
  }
  return result;
}

short int forward_func(int mousefd, int uinputfd, struct event_data *data, struct input_event *event, struct timetable *t) {
  short int result = 0;
  sendevent(uinputfd, EV_KEY, BTN_FORWARD, data->button);
  // sendevent(uinputfd, EV_SYN, SYN_REPORT, 0);
  return result;
}

short int back_func(int mousefd, int uinputfd, struct event_data *data, struct input_event *event, struct timetable *t) {
  short int result = 0;
  sendevent(uinputfd, EV_KEY, BTN_BACK, data->button);
  // sendevent(uinputfd, EV_SYN, SYN_REPORT, 0);
  return result;
}

short int task_func(int mousefd, int uinputfd, struct event_data *data, struct input_event *event, struct timetable *t) {
  short int result = 0;
  sendevent(uinputfd, EV_KEY, BTN_TASK, data->button);
  // sendevent(uinputfd, EV_SYN, SYN_REPORT, 0);
  return result;
}