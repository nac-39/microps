#include "platform.h"
#include "util.h"
#define INTR_IRQ_SHARED 0x1111

struct irq_entry {
  struct irq_entry *next; // 他のIRQ構造体へのポインタ
  unsigned int irq;       // 割り込み番号(IRQ番号)
  int (*handler)(
      unsigned int irq,
      void *dev); // 割り込みハンドラ（割り込みが生じたときに呼び出す関数
  int flags; // フラグ（INTR_IRQ_SHAREDが指定された場合はIRQ番号を共有可能)
  char name[16]; // デバッグ出力で識別するための名前
  void *dev;     // 割り込みの発生源と鳴るデバイス
};

static struct irq_entry *irqs; // IRQリスト（の先頭を表すポインタ)
static sigset_t sigmask;
static pthread_t tid; // 割り込み処理スレッドのスレッドID
static pthread_barrier_t barrier; // スレッド間の同期のためのバリア

int intr_request_irq(unsigned int irq,
                     int (*handler)(unsigned int irq, void *dev), int flags,
                     const char *name, void *dev) {
  struct irq_entry *entry;
  debugf("irq=%u, flags=%d, name=%s", irq, flags, name);
  for (entry = irqs; entry; entry = entry->next) {
    if (entry->irq == irq) {
      // IRQ番号がすでに登録されている場合、IRQ番号の共有が許可されているかどうかチェックする。どちらかが共有を許可していない倍位はエラーを返す。
      if (entry->flags ^ INTR_IRQ_SHARED || flags ^ INTR_IRQ_SHARED) {
        errorf("conflicts with already registered IRQs");
        return -1;
      }
    }
  }
  // 新しいエントリのメモリを確保する
  entry = memory_alloc(sizeof(*entry));
  if (!entry) {
    errorf("memory_alloc() failure");
    return -1;
  }
  // 新しい構造体に値を入れていく
  entry->irq = irq;
  entry->handler = handler;
  entry->flags = flags;
  strncpy(entry->name, name, sizeof(entry->name) - 1);
  entry->dev = dev;
  // irqリストの先頭へ挿入
  entry->next = irqs;
  irqs = entry;
  // シグナル集合へ新しいシグナルを追加
  sigaddset(&sigmask, irq);
  return 0;
}

// 割り込み処理スレッドへシグナルを送信
int intr_raise_irq(unsigned int irq) { return pthread_kill(tid, (int)irq); }

static void *intr_thread(void *arg) {
  int terminate = 0, sig, err;
  struct irq_entry *entry;

  debugf("start...");
  pthread_barrier_wait(&barrier);
  while (!terminate) {
    err = sigwait(&sigmask, &sig);
    if (err) {
      errorf("sigwait() %s", strerror(err));
      break;
    }
    switch (sig) {
    case SIGHUP:
      terminate = 1;
      break;
    default:
      for (entry = irqs; entry; entry = entry->next) {
        if (entry->irq == (unsigned int)sig) {
          debugf("irq=%d, name=%s", entry->irq, entry->name);
          entry->handler(entry->irq, entry->dev);
        }
      }
      break;
    }
  }
  debugf("terminated");
  return NULL;
}

int intr_run(void) {
  int err;
  err = pthread_sigmask(SIG_BLOCK, &sigmask, NULL);
  if (err) {
    errorf("pthread_sigmask() %s", strerror(err));
    return -1;
  }
  err = pthread_create(&tid, NULL, intr_thread, NULL);
  if (err) {
    errorf("pthread_create() %s", strerror(err));
    return -1;
  }
  pthread_barrier_wait(&barrier);
  return 0;
}

void intr_shutdown(void) {
  if (pthread_equal(tid, pthread_self()) != 0) {
    // スレッドが作成されなかった場合
    return;
  }
  pthread_kill(tid, SIGHUP);
  pthread_join(tid, NULL);
}

int intr_init(void) {
  tid = pthread_self();
  pthread_barrier_init(&barrier, NULL, 2);
  sigemptyset(&sigmask);
  sigaddset(&sigmask, SIGHUP);
  return 0;
}
