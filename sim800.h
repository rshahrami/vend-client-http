#ifndef SIM800_H
#define SIM800_H

// ÇÚáÇä ÊÇÈÚ
//unsigned char sim800_basic_init(void);
unsigned char check_signal_quality(void);
unsigned char check_signal_with_restart();

unsigned char check_sim(void);
void gprs_keep_alive(void);

void sim800_restart(void);
unsigned char init_sms(void);
unsigned char init_GPRS(void);
void full_check(void);
uint8_t bringup_gprs_and_sms(void);
uint8_t net_precheck_or_recover(void);

extern char value[16];
extern char buffer[BUFFER_SIZE];
//extern BUFFER_SIZE;

#endif
