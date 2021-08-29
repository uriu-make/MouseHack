#include <errno.h>
#include <fcntl.h>
#include <linux/input.h>
#include <linux/uinput.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <string>

#define DEVICE_NAME "Logitech ERGO M575S"  //機能を変更するデバイス名
#define die(str, args...) \
  do {                    \
    perror(str);          \
    exit(EXIT_FAILURE);   \
  } while (0)

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

int main() {
  //デバイスの捜索
  std::string str, device;
  std::size_t search = std::string::npos;
  do {
    std::ifstream file("/proc/bus/input/devices");
    if (file.fail()) {
      std::cerr << "Failed to open file." << std::endl;
      return -1;
    }
    //デバイスを見つけるか最後まで調べたら捜索終了
    while (getline(file, str) && search == std::string::npos) {
      search = str.find(DEVICE_NAME);
      std::cerr << str << std::endl;
    }
    //デバイスが見つからなければファイルを閉じて500ms待機する
    if (search == std::string::npos) {
      file.close();
      std::cerr << "Device not found." << std::endl;
      
    } else {  //ファイルを見つけたならば割当を調べる
      for (int i = 0; i < 3; i++)
        getline(file, str);
      std::cerr << str << std::endl;
      search = str.find("event");
      device = "/dev/input/" + str.substr(search);
      std::cerr << device;
      device.pop_back();
    }
    usleep(500000);
  } while (search == std::string::npos);
  // char mouse[device.size()+1];
  // sprintf(mouse,"%s",device.c_str());
  // int mousefd = open(mouse, O_RDWR);             //見つけたデバイスを開く
  int mousefd = open(device.c_str(), O_RDWR);             //見つけたデバイスを開く
  // int fd = open("/dev/uinput", O_RDWR);  //マウスとして登録
  int fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);  //マウスとして登録

  create_uinput_device(fd);
  ioctl(mousefd, EVIOCGRAB, 1);                           //マウスを無効化

  sleep(10);
  // while (true) {
  // }
  ioctl(mousefd, EVIOCGRAB, 0);
  close(mousefd);
  // close(fd);
  return 0;
}