/*
 * This include file describes the functions exported by clock.c
 */

#ifndef _CLOCK_H
#define _CLOCK_H

/*
 * Definitions for functions being abstracted out
 */
void sys_tick_handler(void);
void msleep(uint32_t);
void clock_setup(void);

#endif /* generic header protector */

