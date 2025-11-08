#include <stdlib.h>
#include <stdint.h>
#include <glcd.h>
#include <delay.h>
#include <string.h>
#include "common.h"
#include "sim800.h"

//#define BUFFER_SIZE 512
//#define MAX_RETRY   5   // ?????? ????? ????

#define SIGNAL_CHECKS 5

#define LOOP_CHECK 3
//uint8_t site_counter = 0;


//char value[16];

//char at_command[60];
//char sim_number[15];

//char buffer[BUFFER_SIZE];
uint8_t attempts = 0;
uint8_t attemptss = 0;



void sim800_restart(void) {

    glcd_clear();
    glcd_outtextxy(0, 0, "Restarting...");

    // --- 1) ???? HTTP (??? ???? ???) ---

    while (attempts<LOOP_CHECK) {
        //glcd_outtextxy(0, 10, "Waiting HTTPTERM...");
        uart_buffer_reset(); send_at_command("AT+HTTPTERM");
        if (read_until_keyword_keep_all(buffer, BUFFER_SIZE, 2000, ""))
        {
            if (strstr(buffer, "OK") || strstr(buffer, "ERROR")) break;
        }
        attempts++;
    }

    attempts = 0;
    // --- 2) ???? SAPBR (bearer) ---
    while (attempts<LOOP_CHECK) {
        //glcd_outtextxy(0, 10, "Waiting SAPBR=0,1...");
        uart_buffer_reset(); send_at_command("AT+SAPBR=0,1");   // close bearer
        if (read_until_keyword_keep_all(buffer, BUFFER_SIZE, 2000, ""))
        {
            if (strstr(buffer, "OK") || strstr(buffer, "ERROR")) break;
        }
        attempts++;
    }

    attempts = 0;

    while (attempts<LOOP_CHECK) {
        //glcd_outtextxy(0, 10, "Checking SAPBR closed...");
        uart_buffer_reset(); send_at_command("AT+SAPBR=2,1");   // close bearer
        if (read_until_keyword_keep_all(buffer, BUFFER_SIZE, 2000, "SAPBR"))
        {
            if (strstr(buffer, "0.0.0.0") || strstr(buffer, ",0,") || strstr(buffer, ",3,")) break;
        }
        attempts++;
    }

    attempts = 0;


    while (attempts<LOOP_CHECK) {
        //glcd_outtextxy(0, 10, "Waiting CGATT=0...");
        uart_buffer_reset(); send_at_command("AT+CGATT=0");   // close bearer
        if (read_until_keyword_keep_all(buffer, BUFFER_SIZE, 4000, "OK")) break;
        attempts++;
    }


    attempts=0;
    while (attempts<LOOP_CHECK) {
        uart_buffer_reset(); send_at_command("AT+CFUN=1,1");
        read_until_keyword_keep_all(buffer, BUFFER_SIZE, 2000, "OK"); // (E??I?? C?? ??C?I)
        //glcd_outtextxy(0, 10, "Rebooting, AT ...");
        while (attemptss<5) {
            uart_buffer_reset();
            send_at_command("AT");
            if (read_until_keyword_keep_all(buffer, BUFFER_SIZE, 2000, "OK")) {
                goto after_reboot;
            }
            // C?? I?C?E? ?? E?C?? C???C ??C? ????E ? delay ???C???E? E?C??
            //delay_ms(50);
            attemptss++;
        }
        attemptss=0;
        attempts++;
    }


after_reboot:

    //glcd_clear();
    attempts=0;
    // 6) Echo/Errors/URC SMS IC??O (E?C? E???? ?C?)
    while (attempts<LOOP_CHECK) {
        uart_buffer_reset(); send_at_command("ATE0");
        if (read_until_keyword_keep_all(buffer, BUFFER_SIZE, 2000, "OK")) break;
        attempts++;
    }
    attempts = 0;
    glcd_outtextxy(0, 12, "> Restart Done!");
    delay_ms(100);
}


unsigned char check_sim(void) {
    while(attempts<LOOP_CHECK){
        uart_buffer_reset(); send_at_command("AT+CPIN?");
        if (read_until_keyword_keep_all(buffer, BUFFER_SIZE, 5000, "CPIN")) {
            if (strstr(buffer, "READY")) break;
        }
        attempts++;
    }

    attempts=0;

    while(attempts<LOOP_CHECK){
        uart_buffer_reset(); send_at_command("AT+CREG?");
        if (read_until_keyword_keep_all(buffer, BUFFER_SIZE, 5000, "CREG")) {
            if (extract_field_after_keyword(buffer, "+CREG:", 1, value, sizeof(value))) {
                if (atoi(value) == 1) break;
            }
        }
        attempts++;
    }
    attempts=0;
    glcd_clear();
    glcd_outtextxy(0, 0, "> Network  OK!");

    //delay_ms(50);

    return 1;
}



unsigned char check_signal_quality(void)
{
    int csq;

    while(attempts<LOOP_CHECK){
        uart_buffer_reset(); send_at_command("AT+CSQ");
        if (read_until_keyword_keep_all(buffer, BUFFER_SIZE, 5000, "CSQ")) {
            if (extract_field_after_keyword(buffer, "+CSQ:", 0, value, sizeof(value))) {
                csq = atoi(value);
                if (csq == 99) return 0;
                if (csq < 5) return 0;
                glcd_outtextxy(0, 12, "> Quality  OK!");

                return 1;
            }
        }
        attempts++;
    }

    attempts=0;

    return 0;
}


unsigned char init_GPRS(void)
{
    //uint8_t attempts = 0;
    char at_command[80];

//    glcd_clear();
//    glcd_outtextxy(0, 0, "Connecting to GPRS...");

    uart_buffer_reset(); send_at_command("AT+SAPBR=3,1,\"Contype\",\"GPRS\"");
    (void)read_until_keyword_keep_all(buffer, BUFFER_SIZE, 1000, "OK");

    sprintf(at_command, "AT+SAPBR=3,1,\"APN\",\"%s\"", APN);
    uart_buffer_reset(); send_at_command(at_command);
    (void)read_until_keyword_keep_all(buffer, BUFFER_SIZE, 1000, "OK");

    uart_buffer_reset(); send_at_command("AT+SAPBR=1,1");
    (void)read_until_keyword_keep_all(buffer, BUFFER_SIZE, 1000, "OK");


//    glcd_clear();
//    glcd_outtextxy(0, 0, "Fetching IP...");

    while (attempts < LOOP_CHECK) {
        uart_buffer_reset(); send_at_command("AT+SAPBR=2,1");
        if (read_until_keyword_keep_all(buffer, BUFFER_SIZE, 5000, "SAPBR")) {
            // ??????? status (???? ??? ??? ?? +SAPBR:)
            if (extract_field_after_keyword(buffer, "+SAPBR:", 1, value, sizeof(value))) {
                if (atoi(value) == 1) {
                    // ??? status=1 ???? ???? ??? ???? IP ?? ???? ???
                    if (extract_field_after_keyword(buffer, "+SAPBR:", 2, value, sizeof(value))) {

//                        glcd_clear();
//                        glcd_outtextxy(0, 0, value);
//                        delay_ms(300);

                        glcd_outtextxy(0, 22, "> GPRS     OK!");

                        return 1; // ????
                    }
                }
            }
        }
        attempts++;
    }

    attempts=0;
    glcd_clear();
    glcd_outtextxy(0, 0, "No IP");
    return 0; // ??????
}


void http_connect(void)
{
//    glcd_clear();
//    glcd_outtextxy(0, 0, "Setting HTTP Mode...");

    // init HTTP   CID
    uart_buffer_reset(); send_at_command("AT+HTTPINIT");
    (void)read_until_keyword_keep_all(buffer, sizeof(buffer), 2000, "OK");

    uart_buffer_reset(); send_at_command("AT+HTTPPARA=\"CID\",1");
    (void)read_until_keyword_keep_all(buffer, sizeof(buffer), 2000, "OK");

    glcd_outtextxy(0, 32, "> HTTP     OK!");



}


unsigned char init_sms(void)
{
    //glcd_clear();
    //glcd_outtextxy(0, 0, "Setting SMS Mode...");

    //send_at_command("AT+CFUN=1");              (void)read_until_keyword_keep_all(buffer, BUFFER_SIZE, 100, "OK");
    send_at_command("AT+CSCLK=0");             (void)read_until_keyword_keep_all(buffer, BUFFER_SIZE, 100, "OK");
    send_at_command("AT+CMGF=1");              (void)read_until_keyword_keep_all(buffer, BUFFER_SIZE, 100, "OK");

    uart_buffer_reset();
    send_at_command("AT+CNMI=2,2,0,0,0");      (void)read_until_keyword_keep_all(buffer, BUFFER_SIZE, 100, "OK");

    send_at_command("AT+CMGDA=\"DEL ALL\"");   (void)read_until_keyword_keep_all(buffer, BUFFER_SIZE, 100, "OK");

    glcd_outtextxy(0, 42, "> SMS      OK!");

    delay_ms(200);
    return 1;
}


uint8_t bringup_gprs_and_sms(void)
{
    while (1) {

        check_sim();
        // OE??
        if (!check_signal_quality()) { sim800_restart(); continue; }

        // PDP/IP
        //if (!soft_bringup_ip()) { sim800_restart(); continue; }
        if (!init_GPRS()) { sim800_restart(); continue; }

        // TCP
        //if (!tcp_connect(APP_HOST, APP_PORT)) { sim800_restart(); continue; }
        http_connect();
        // SMS ON
        init_sms();

        // CNMI E??? init_sms ??C? OI?   A?CI??C??
        return 1;
    }
}


// ???????????: 1 ???? ??????? ok ? ??????? gprs_keep_alive ?? ??? ?????
// 0 ???? ??????? ????? ??/????? ????? ??? ? ??? ???? keep-alive ?? ?? ??.
uint8_t checking(void)
{

    glcd_clear();
    glcd_outtextxy(0, 0, "Checking ...");
    // 0) AT ????
    uart_buffer_reset(); send_at_command("AT");
    if (!read_until_keyword_keep_all(buffer, BUFFER_SIZE, 500, "OK")) {
        //glcd_outtextxy(0, 0, "No AT -> Reboot");
        //sim800_restart();
        bringup_gprs_and_sms();
        return 0;
    }

    // 1) ????? attach
    uart_buffer_reset(); send_at_command("AT+CGATT?");
    if (read_until_keyword_keep_all(buffer, BUFFER_SIZE, 1500, "+CGATT:")) {
        if (extract_field_after_keyword(buffer, "+CGATT:", 0, value, sizeof(value))) {
            if (atoi(value) != 1) {
                //glcd_outtextxy(0, 0, "CGATT=0 -> Recover");
                bringup_gprs_and_sms();
                return 0;
            }
        }
    } else {
        //glcd_outtextxy(0, 0, "CGATT? timeout");
        bringup_gprs_and_sms();
        return 0;
    }

    // 2) ????? bearer (SAPBR)
    uart_buffer_reset(); send_at_command("AT+SAPBR=2,1");
    if (read_until_keyword_keep_all(buffer, BUFFER_SIZE, 1500, "+SAPBR:")) {
        // +SAPBR: 1,<stat>,"<ip>"
        if (extract_field_after_keyword(buffer, "+SAPBR:", 1, value, sizeof(value))) {
            //int stat = atoi(value);
            if (atoi(value) != 1) {
                // ???? ??? ???? ??? ???? bearer
                uart_buffer_reset(); send_at_command("AT+SAPBR=1,1");
                read_until_keyword_keep_all(buffer, BUFFER_SIZE, 3000, "OK");

                // ???????? ?????
                uart_buffer_reset(); send_at_command("AT+SAPBR=2,1");
                if (read_until_keyword_keep_all(buffer, BUFFER_SIZE, 1500, "+SAPBR:") &&
                    extract_field_after_keyword(buffer, "+SAPBR:", 1, value, sizeof(value)) &&
                    atoi(value) == 1) {
                    return 1; // ok ??? keep-alive ????? ???
                }

                //glcd_outtextxy(0, 0, "Bearer fail -> Recover");
                bringup_gprs_and_sms();
                return 0;
            }
            // ???????: IP ?? ?? ?? ???? ?? 0.0.0.0 ?????
            if (extract_field_after_keyword(buffer, "+SAPBR:", 2, value, sizeof(value))) {
                if (strstr(value, "0.0.0.0")) {
                    //glcd_outtextxy(0, 0, "No IP -> Recover");
                    bringup_gprs_and_sms();
                    return 0;
                }
            }
            return 1; // bearer ok? IP ?? ????? ??? keep-alive
        }
    }
    attempts=0;
    // ??? ???? ???/???????? ??
    //glcd_outtextxy(0, 0, "SAPBR? timeout -> Recover");
    bringup_gprs_and_sms();
    return 0;
}


void http_keep_alive(void) {
    glcd_clear();
    glcd_outtextxy(0, 0, "Keep_Alive ...");

    uart_buffer_reset(); send_at_command("AT+HTTPPARA=\"URL\",\"http://www.google.com\"");
    (void)read_until_keyword_keep_all(buffer, BUFFER_SIZE, 500, "OK");


    uart_buffer_reset(); send_at_command("AT+HTTPACTION=2"); // 0=GET
    (void)read_until_keyword_keep_all(buffer, BUFFER_SIZE, 500, "OK");

//    if(site_counter==0)
//        uart_buffer_reset(); send_at_command("AT+HTTPPARA=\"URL\",\"http://www.google.com\"");
//        (void)read_until_keyword_keep_all(buffer, BUFFER_SIZE, 500, "OK");
//    if(site_counter==1)
//        uart_buffer_reset(); send_at_command("AT+HTTPPARA=\"URL\",\"http://www.google.com\"");
//        (void)read_until_keyword_keep_all(buffer, BUFFER_SIZE, 500, "OK");
//    if(site_counter==2)
//        uart_buffer_reset(); send_at_command("AT+HTTPPARA=\"URL\",\"http://www.google.com\"");
//        (void)read_until_keyword_keep_all(buffer, BUFFER_SIZE, 500, "OK");

}