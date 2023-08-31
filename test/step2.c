#include "driver/dummy.h"

static volatile sig_atomic_t terminate;
static void on_signal(int s) {
  (void)s;
  terminate = 1;
}
uint8_t *test_data;

int main(int argc, char *argv[]) {
  struct net_device *dev;
  signal(SIGINT, on_signal); // シグナルハンドラの設定(^Cで終了できるように)
  if (net_init() == -1) {
    errorf("net_init() failure");
    return -1;
  }
  dev = dummy_init();
  if (!dev) {
    errorf("dummy_init() failure");
    return -1;
  }
  if (net_run() == -1) {
    errorf("net_run() failure");
    return -1;
  }
  while (!terminate) {
    if (net_device_output(dev, 0x0800, test_data, sizeof(test_data), NULL) ==
        -1) {
      errorf("net_device_outoput() failure");
      break;
    }
  }
  net_shutdown();
  return 0;
}
