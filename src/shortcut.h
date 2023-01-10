#ifndef SHORTCUT_H
#define SHORTCUT_H
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
#include <filesystem>

#define LEFT 0
#define RIGHT 1
#define MIDDLE 2
#define SIDE 3
#define EXTRA 4
#define FORWARD 5
#define BACK 6
#define TASK 7
#define DEVICE_NAME "M575"  //機能を変更するデバイス名
#define TOUCHPAD0 "0X53 0X59 0X4E 0X50 06CB:CED3 Mouse"
#define TOUCHPAD1 "0X53 0X59 0X4E 0X50 06CB:CED3 Touchpad"

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

struct event_data {
  char button;
  char button_old;
  suseconds_t timer;
};
std::string SearchDevice(std::string device);
void create_uinput_device(int fd);
//イベントの受信
short int readevent(int fd, struct input_event *event, struct timetable *t);
//イベントの送信
void sendevent(int output, int type, int code, int value);

short int left_func(int mousefd, int uinputfd, struct event_data *data, struct input_event *event, struct timetable *t);
short int right_func(int mousefd, int uinputfd, struct event_data *data, struct input_event *event, struct timetable *t);
short int middle_func(int mousefd, int uinputfd, struct event_data *data, struct input_event *event, struct timetable *t);
short int side_func(int mousefd, int uinputfd, struct event_data *data, struct input_event *event, struct timetable *t);
short int extra_func(int mousefd, int uinputfd, struct event_data *data, struct input_event *event, struct timetable *t);
short int forward_func(int mousefd, int uinputfd, struct event_data *data, struct input_event *event, struct timetable *t);
short int back_func(int mousefd, int uinputfd, struct event_data *data, struct input_event *event, struct timetable *t);
short int task_func(int mousefd, int uinputfd, struct event_data *data, struct input_event *event, struct timetable *t);
#endif
