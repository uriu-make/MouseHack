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
  fds.events = POLLIN;
  poll(&fds, nfds, 1);          //マウスのイベントが読み取り可能かタイムアウト付きで確認
  if (fds.revents == POLLIN) {  //読み取り可能ならば読み込む
    read(fd, event, sizeof(input_event));
  }
  gettimeofday(&time, NULL);  // 1ループの時間計測
  t->current_time = time.tv_sec * 1000000 + time.tv_usec;
  t->runtime = t->current_time - t->old_time;
  t->old_time = t->current_time;
  // }
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