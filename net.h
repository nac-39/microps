#include <stddef.h>
#include <stdint.h>

#include "platform.h"
#include "util.h"

#define NET_DEVICE_ADDR_LEN 0
#define IFNAMSIZ 0
#define NET_DEVICE_FLAG_UP 0x0001
#define NET_DEVICE_TYPE_XXX
#define NET_DEVICE_IS_UP(x) ((x)->flags & NET_DEVICE_FLAG_UP)
#define NET_DEVICE_STATE(x) (NET_DEVICE_IS_UP(x) ? "up" : "down")
#define NET_DEVICE_TYPE_DUMMY 0x001
#define DUMMY_MTU UINT16_MAX

struct net_device {
  struct net_device *next;
  unsigned int index;
  char name[IFNAMSIZ];
  uint16_t type;
  uint16_t
      mtu; // デバイスのMTU(Maximum Transmission Unit)
           // そのデータリンクで一度に送信可能なデータの最大サイズ
           // そのデータリンクによってMTUの値は異なる
           // MTUを超えるサイズのデータは送信できないので、上位レイヤーでサイズを調整する必要がある
  uint16_t flags; // 各種フラグ
  uint16_t hlen;  // header length
  uint16_t alen;  // addres length
  uint8_t addr[NET_DEVICE_ADDR_LEN];
  union {
    uint8_t peer[NET_DEVICE_ADDR_LEN];
    uint8_t broadcast[NET_DEVICE_ADDR_LEN];
  };
  struct net_device_ops
      *ops; // デバイっすドライバに実装されている関数が設定されたstruct
            // net_device_ops へのポインタ
  void *prev; // デバイスドライバが使うプライベートなデータへのポインタ
};

struct net_device_ops {
  int (*open)(struct net_device *dev);
  int (*close)(struct net_device *dev);
  int (*transmit)(struct net_device *dev, uint16_t type, const uint8_t *data,
                  size_t len, const void *dst);
};

struct net_device *net_device_alloc(void);
int net_device_register(struct net_device *dev);
int net_init(void);
int net_device_output(struct net_device *dev, uint16_t type,
                      const uint8_t *data, size_t len, const void *dst);
void net_shutdown(void);
int net_run(void);
