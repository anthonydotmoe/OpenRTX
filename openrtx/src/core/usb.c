#include <pthread.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "core/usb.h"
#include "interfaces/delays.h"
#include "subprojects/tinyusb/src/tusb.h"

#define USB_LOG_MSG_MAX 128
typedef struct
{
    uint16_t len;
    char     data[USB_LOG_MSG_MAX];
} UsbLogMsg;

#define USB_LOG_Q_MAX 32
typedef struct {
    UsbLogMsg       buf[USB_LOG_Q_MAX];
    unsigned        head;
    unsigned        tail;
    pthread_mutex_t mutex;
    pthread_cond_t  cv;
} UsbLogQueue;

static UsbLogQueue g_usb_log_q = {
    .buf   = {0},
    .head  = 0,
    .tail  = 0,
    .mutex = PTHREAD_MUTEX_INITIALIZER,
    .cv    = PTHREAD_COND_INITIALIZER,
};

bool usb_log_send(const char *s, size_t len)
{
    if (len > USB_LOG_MSG_MAX) len = USB_LOG_MSG_MAX;

    pthread_mutex_lock(&g_usb_log_q.mutex);

    unsigned next_head = (g_usb_log_q.head + 1) % USB_LOG_Q_MAX;
    if (next_head == g_usb_log_q.tail)
    {
        // queue full
        // TODO: drop or block?
        pthread_mutex_unlock(&g_usb_log_q.mutex);
        return false;
    }

    UsbLogMsg *slot = &g_usb_log_q.buf[g_usb_log_q.head];
    slot->len = (uint16_t)len; 
    memcpy(slot->data, s, len);

    g_usb_log_q.head = next_head;

    pthread_cond_signal(&g_usb_log_q.cv);
    pthread_mutex_unlock(&g_usb_log_q.mutex);

    return true;
}

static bool usb_log_recv(UsbLogMsg *out)
{
    pthread_mutex_lock(&g_usb_log_q.mutex);

    while (g_usb_log_q.head == g_usb_log_q.tail)
        pthread_cond_wait(&g_usb_log_q.cv, &g_usb_log_q.mutex);
    
    *out = g_usb_log_q.buf[g_usb_log_q.tail];
    g_usb_log_q.tail = (g_usb_log_q.tail + 1) % USB_LOG_Q_MAX;

    pthread_mutex_unlock(&g_usb_log_q.mutex);
    return true;
}

static bool usb_log_try_recv(UsbLogMsg *out)
{
    pthread_mutex_lock(&g_usb_log_q.mutex);

    if (g_usb_log_q.head == g_usb_log_q.tail) {
        pthread_mutex_unlock(&g_usb_log_q.mutex);
        return false;
    }

    *out = g_usb_log_q.buf[g_usb_log_q.tail];
    g_usb_log_q.tail = (g_usb_log_q.tail + 1) % USB_LOG_Q_MAX;
    pthread_mutex_unlock(&g_usb_log_q.mutex);
    return true;
}

static void vcom_write(UsbLogMsg *msg)
{
    const uint8_t *data = (const uint8_t *)&msg->data;
    const size_t len = (const size_t)msg->len;
    size_t written = 0;
    uint16_t timeout = 0;

    if (!tud_cdc_connected()) {
        return;
    }

    while (written < len)
    {
        uint32_t avail;
        if ((avail = tud_cdc_write_available()) == 0)
        {
            if (timeout++ > 500) return;
            delayMs(1);
            continue;
        }

        uint32_t chunk = (uint32_t)(len - written);
        if (chunk > avail) chunk = avail;

        written += tud_cdc_write(data + written, chunk);
    }

    tud_cdc_write_flush();
}

void *usb_threadfunc(void *arg)
{
    (void) arg;
    UsbLogMsg msg;
    long long next = getTick() + 1;

    for (;;) {
        tud_task();

        while (usb_log_try_recv(&msg))
        {
            vcom_write(&msg);
        }

        sleepUntil(next);
        next += 1;
    }
}
