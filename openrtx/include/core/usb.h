#ifndef CORE_USB_H
#define CORE_USB_H

#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Thread function for threads.c */
void *usb_threadfunc(void *arg);

bool usb_log_send(const char *s, size_t len);

#ifdef __cplusplus
}
#endif

#endif /* CORE_USB_H */