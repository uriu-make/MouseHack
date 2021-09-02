#include <errno.h>
#include <fcntl.h>
#include <linux/input.h>
#include <linux/uinput.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <string>

#define LEFT 0
#define RIGHT 1
#define MIDDLE 2
#define SIDE 3
#define EXTRA 4
#define FORWARD 5
#define BACK 6
#define TASK 7
#define DEVICE_NAME "Logitech ERGO M575S"  //機能を変更するデバイス名
#define die(str, args...) \
  do {                    \
    perror(str);          \
    exit(EXIT_FAILURE);   \
  } while (0)

struct timetable {  //時間の管理用
  suseconds_t current_time;
  suseconds_t old_time;
  suseconds_t runtime;
};

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
  fds.events = POLLIN;
  poll(&fds, nfds, 1);
  if (fds.revents == POLLIN)  //読み取り可能ならば読み込む
    read(fd, event, sizeof(input_event));
  gettimeofday(&time, NULL);  //1ループの時間計測
  t->current_time = time.tv_sec * 1000000 + time.tv_usec;
  t->runtime = t->current_time - t->old_time;
  t->old_time = t->current_time;
  return result;
}
//イベントの送信
void sendevent(int output, int type, int code, int value) {
  struct input_event event;
  event.type = type;
  event.code = code;
  event.value = value;
  gettimeofday(&event.time, NULL);
  write(output, &event, sizeof(event));
}

int main() {
  std::string str, device;
  std::size_t search = std::string::npos;
  while (true) {
    //デバイスの捜索
    do {
      std::ifstream file("/proc/bus/input/devices");
      if (file.fail()) {
        std::cerr << "Failed to open file." << std::endl;
        return -1;
      }
      //デバイスを見つけるか最後まで調べたら捜索終了
      while (getline(file, str) && search == std::string::npos) {
        search = str.find(DEVICE_NAME);
      }
      //デバイスが見つからなければファイルを閉じて500ms待機する
      if (search == std::string::npos) {
        file.close();
      } else {  //ファイルを見つけたならば割当を調べる
        for (int i = 0; i < 3; i++)
          getline(file, str);
        search = str.find("event");
        device = "/dev/input/" + str.substr(search);
        // std::cerr << device << std::endl;
        device.pop_back();
      }
      usleep(500000);
    } while (search == std::string::npos);

    int mousefd = open(device.c_str(), O_RDWR);  //見つけたデバイスを開く
    int fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    ioctl(mousefd, EVIOCGRAB, 1);  //マウスを無効化

    ioctl(fd, UI_SET_EVBIT, EV_REL);  //使用するマウスの機能
    ioctl(fd, UI_SET_RELBIT, REL_X);
    ioctl(fd, UI_SET_RELBIT, REL_Y);
    ioctl(fd, UI_SET_RELBIT, REL_WHEEL);
    ioctl(fd, UI_SET_EVBIT, EV_KEY);  //使用するキーとボタン
    ioctl(fd, UI_SET_KEYBIT, BTN_LEFT);
    ioctl(fd, UI_SET_KEYBIT, BTN_RIGHT);
    ioctl(fd, UI_SET_KEYBIT, BTN_MIDDLE);
    ioctl(fd, UI_SET_KEYBIT, KEY_BACK);
    ioctl(fd, UI_SET_KEYBIT, KEY_FORWARD);
    ioctl(fd, UI_SET_KEYBIT, KEY_TAB);
    ioctl(fd, UI_SET_KEYBIT, KEY_LEFTALT);
    ioctl(fd, UI_SET_KEYBIT, KEY_LEFTCTRL);
    ioctl(fd, UI_SET_KEYBIT, KEY_D);
    ioctl(fd, UI_SET_KEYBIT, KEY_0);
    create_uinput_device(fd);  //マウスとして登録

    char button[8] = {0};
    char button_old[8] = {0};
    suseconds_t timer[8] = {0};
    short int result = 0;
    struct timeval time;
    gettimeofday(&time, NULL);
    struct input_event event, output;
    struct timetable t;

    while (result == 0) {
      result = readevent(mousefd, &event, &t);
      if (event.type == EV_KEY) {  //ボタンの入力ならばボタンの種類ごとに処理
        switch (event.code) {
          // case BTN_LEFT:  //各ボタンの状態とイベント以前の状態を記録しイベントが発生してからの経過時間を初期化
          //   button_old[0] = button[0];
          //   button[0] = event.value;
          //   timer[0] = 0;
          //   break;
          // case BTN_RIGHT:
          //   button_old[1] = button[1];
          //   button[1] = event.value;
          //   timer[1] = 0;
          //   break;
          case BTN_MIDDLE:
            button_old[MIDDLE] = button[MIDDLE];
            button[MIDDLE] = event.value;
            timer[MIDDLE] = 0;
            break;
          case BTN_SIDE:
            button_old[SIDE] = button[SIDE];
            button[SIDE] = event.value;
            timer[SIDE] = 0;
            break;
          case BTN_EXTRA:
            button_old[EXTRA] = button[EXTRA];
            button[EXTRA] = event.value;
            timer[EXTRA] = 0;
            break;
          case BTN_FORWARD:
            button_old[FORWARD] = button[FORWARD];
            button[FORWARD] = event.value;
            timer[FORWARD] = 0;
            break;
          case BTN_BACK:
            button_old[BACK] = button[BACK];
            button[BACK] = event.value;
            timer[BACK] = 0;
            break;
          case BTN_TASK:
            button_old[TASK] = button[TASK];
            button[TASK] = event.value;
            timer[TASK] = 0;
            break;
          default:  //以上に当てはまらなければそのままイベントを送信
            write(fd, &event, sizeof(event));
            break;
        }
      } else {  //その他のイベントならばそのまま送信
        write(fd, &event, sizeof(event));
      }
      if (button[EXTRA] == 1) {  //EXTRAボタンならばAlt+Tabを送信
        sendevent(fd, EV_KEY, KEY_LEFTALT, 1);
        sendevent(fd, EV_KEY, KEY_TAB, 1);
        sendevent(fd, EV_SYN, SYN_REPORT, 0);
        timer[4] = 0;
      } else if (button[EXTRA] == 0 && button_old[EXTRA] == 1 && timer[EXTRA] < 1000000) {  //EXTRAボタンを離しても1秒以内ならばAltキーを送信
        sendevent(fd, EV_KEY, KEY_LEFTALT, 1);
        sendevent(fd, EV_KEY, KEY_TAB, 0);
        sendevent(fd, EV_SYN, SYN_REPORT, 0);
        timer[EXTRA] += t.runtime;
        button_old[EXTRA] = 1;
      } else {  //キー入力終了
        sendevent(fd, EV_KEY, KEY_LEFTALT, 0);
        sendevent(fd, EV_KEY, KEY_TAB, 0);
        sendevent(fd, EV_SYN, SYN_REPORT, 0);
        button_old[EXTRA] = 0;
        timer[EXTRA] = 0;
      }
      if (button[SIDE] == 1 && button_old[SIDE] == 0) {  //SIDEボタンを押したならばCtrl+Alt+Dを送信
        sendevent(fd, EV_KEY, KEY_LEFTCTRL, 1);
        sendevent(fd, EV_KEY, KEY_LEFTALT, 1);
        sendevent(fd, EV_KEY, KEY_D, 1);
        sendevent(fd, EV_SYN, SYN_REPORT, 0);
      } else if (button[SIDE] == 0 && button_old[SIDE] == 1) {
        sendevent(fd, EV_KEY, KEY_LEFTCTRL, 0);
        sendevent(fd, EV_KEY, KEY_LEFTALT, 0);
        sendevent(fd, EV_KEY, KEY_D, 0);
        sendevent(fd, EV_SYN, SYN_REPORT, 0);
        button_old[SIDE] = 0;
      }
      if (button[MIDDLE] == 1 && button_old[MIDDLE] == 0) {  //ミドルクリックでジェスチャー
        int x = 0, y = 0, wheel = 0;
        while (button[MIDDLE] == 1 && result == 0) {
          result = readevent(mousefd, &event, &t);
          if (event.type == EV_REL) {
            switch (event.code) {
              case REL_X:
                x += event.value;
                break;
              case REL_Y:
                y += event.value;
                break;
            }
          } else if (event.type == EV_KEY && event.code == BTN_MIDDLE) {
            button_old[MIDDLE] = button[MIDDLE];
            button[MIDDLE] = event.value;
          }
          if (y <= -75 && abs(x) < 50) {  //上下方向に動かすとCtrl+マウスホイール上下に置き換え
            wheel = 1;
            y = 0;
            x = 0;
            sendevent(fd, EV_KEY, KEY_LEFTCTRL, 1);
            sendevent(fd, EV_REL, REL_WHEEL, wheel);
            sendevent(fd, EV_SYN, SYN_REPORT, 0);
          } else if (y >= 75 && abs(x) < 50) {
            wheel = -1;
            y = 0;
            x = 0;
            sendevent(fd, EV_KEY, KEY_LEFTCTRL, 1);
            sendevent(fd, EV_REL, REL_WHEEL, wheel);
            sendevent(fd, EV_SYN, SYN_REPORT, 0);
          } else if (x >= 50 && abs(y) < 50) {  //左右方向の移動があれば進む/戻るに置換
            y = 0;
            x = 0;
            sendevent(fd, EV_KEY, KEY_FORWARD, 1);
            sendevent(fd, EV_SYN, SYN_REPORT, 0);
          } else if (x <= -50 && abs(y) < 50) {
            y = 0;
            x = 0;
            sendevent(fd, EV_KEY, KEY_BACK, 1);
            sendevent(fd, EV_SYN, SYN_REPORT, 0);
          }
        }
        if (x == 0 && y == 0) {  //ミドルクリック終了までに操作がなければ拡大率を初期化(Ctrl+0)
          sendevent(fd, EV_KEY, KEY_LEFTCTRL, 1);
          sendevent(fd, EV_KEY, KEY_0, 1);
          sendevent(fd, EV_SYN, SYN_REPORT, 0);
        }
      } else if (button[MIDDLE] == 0 && button_old[MIDDLE] == 1) {
        sendevent(fd, EV_KEY, KEY_LEFTCTRL, 0);
        sendevent(fd, EV_KEY, KEY_FORWARD, 0);
        sendevent(fd, EV_KEY, KEY_BACK, 0);
        sendevent(fd, EV_KEY, KEY_0, 0);
        sendevent(fd, EV_SYN, SYN_REPORT, 0);
        button_old[MIDDLE] = 0;
      }
    }
    ioctl(mousefd, EVIOCGRAB, 0);
    close(mousefd);
    close(fd);
  }
  return 0;
}