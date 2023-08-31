/* NOTE: if you want to add/delete the entries after net_run(), you need to
 * protect these lists with a mutex. */
#include "net.h"
#include "platform.h"
#include "util.h"

static struct net_device *devices;

struct net_device *net_device_alloc(void) {
  struct net_device *dev;

  dev = memory_alloc(sizeof(*dev));
  if (!dev) {
    errorf("memory_alloc() failure");
  }
  return dev;
}

/* NOTE: must not be call after net_run() */
int net_device_register(struct net_device *dev) {
  static unsigned int index = 0;
  dev->index = index++; // デバイスのインデックス番号を設定
  snprintf(dev->name, sizeof(dev->name), "net%d",
           dev->index); // デバイス名を生成
  dev->next = devices;
  devices = dev;
  infof("registerd, dev=%s, type=0x%04x", dev->name, dev->type);
  return 0;
}

static int net_device_open(struct net_device *dev) {
  if (NET_DEVICE_IS_UP(dev)) {
    errorf("already opened, dev=%s", dev->name);
    return -1;
  }
  if (dev->ops->open) {
    if (dev->ops->open(dev) == -1) {
      errorf("failure, dev=%s", dev->name);
      return -1;
    }
  }
  dev->flags |= NET_DEVICE_FLAG_UP;
  infof("dev=%s, state=%s", dev->name, NET_DEVICE_STATE(dev));
  return 0;
}
static int net_device_close(struct net_device *dev) {
  // デバイスの状態を確認し、UP状態でない場合はerrorを返す
  if (!NET_DEVICE_IS_UP(dev)) {
    errorf("already closeed, dev=%s", dev->name);
    return -1;
  }
  if (dev->ops->close) {
    if (dev->ops->close(dev) == -1) {
      errorf("failure, dev=%s", dev->name);
      return -1;
    }
  }
<<<<<<< HEAD
  dev->flags &= ~NET_DEVICE_FLAG_UP; // upフラグを落とす
=======
  dev->flags &= -NET_DEVICE_FLAG_UP; // upフラグを落とす
>>>>>>> 5213a178c0f999843a4aeb1624345cef208cb11f
  infof("dev=%s, state=%s", dev->name, NET_DEVICE_STATE(dev));
  return 0;
}

// デバイスへの出力
int net_device_output(struct net_device *dev, uint16_t type,
                      const uint8_t *data, size_t len, const void *dst) {
  if (!NET_DEVICE_IS_UP(dev)) {
    errorf("not opened, dev=%s", dev->name);
    return -1;
  }
  if (len > dev->mtu) {
    errorf("too long, dev=%s, mtu=%u, len=%zu", dev->name, dev->mtu, len);
    return -1;
  }
  debugf("dev=%s, type=0x%04x, len=%zu", dev->name, type, len);
  debugdump(data, len);
  if (dev->ops->transmit(dev, type, data, len, dst) == -1) {
    errorf("device transmit failure, dev=%s, len=%zu", dev->name, len);
    return -1;
  }
}

// デバイスが受信したパケットをプロトコルスタックに渡す関数
// プロトコルスタックへのデータの入り口であり、デバイスドライバから呼び出されることを想定している
int net_input_handler(uint16_t type, const uint8_t *data, size_t len,
                      struct net_device *dev) {
  debugf("dev=%s, type=0x%04x, len=%zu", dev->name, type, len);
  debugdump(data, len);
  return 0;
}

// 登録済みの全デバイスをオープン
int net_run(void) {
  struct net_device *dev;
<<<<<<< HEAD
  // 割り込み機構の起動
  if (intr_run() == 1) {
    errorf("intr_run() failure");
    return -1;
  }
=======
>>>>>>> 5213a178c0f999843a4aeb1624345cef208cb11f
  debugf("open all devices...");
  for (dev = devices; dev; dev = dev->next) {
    net_device_open(dev);
  }
  debugf("runnning...");
  return 0;
}

// 登録済みの全デバイスをクローズ
void net_shutdown(void) {
  struct net_device *dev;
  intr_shutdown(); // 割り込み機構の終了
  debugf("close all devices...");
  for (dev = devices; dev; dev = dev->next) {
    net_device_close(dev);
  }
  debugf("shutting down");
}

int net_init(void) {
  // 割り込み機構の初期化
  if (intr_init() == -1) {
    errorf("intr_init() failure");
    return -1;
  }
  infof("initialized");
  return 0;
}
