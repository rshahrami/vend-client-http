#include "common.h"
#include <string.h>
#include <delay.h>
#include <mega64a.h>
#include <glcd.h>

#define glcd_pixel(x, y, color) glcd_setpixel(x, y)
#define read_flash_byte(p) (*(p))

// --- ÇÑÓÇá ÏÓÊæÑ AT ---
void send_at_command(char *command)
{
    printf("%s\r\n", command);
}

// --- Ç˜ ˜ÑÏä ÈÇİÑ USART0 ---
//void uart_flush0(void)
//{
//    unsigned char dummy;
//
//    // Ç˜ ˜ÑÏä ÑÌíÓÊÑ ÓÎÊÇİÒÇÑí
//    while (UCSR0A & (1<<RXC0)) {
//        dummy = UDR0;
//    }
////
////    // Ç˜ ˜ÑÏä ÈÇİÑ äÑãÇİÒÇÑí
////    rx_wr_index0 = rx_rd_index0 = 0;
////    rx_counter0 = 0;
////    rx_buffer_overflow0 = 0;
//}


void uart_buffer_reset(void) {
    rx_wr_index0 = rx_rd_index0 = 0;
    rx_counter0 = 0;
    rx_buffer_overflow0 = 0;
}


//unsigned char read_serial_timeout_simple(char* buffer, int buffer_size, unsigned long timeout_ms) {
//    int i = 0;
//    unsigned long elapsed = 0;
//
//    // ÈÇİÑ ãÍáí ÑÇ ÕİÑ ãí˜äíã
//    memset(buffer, 0, buffer_size);
//
//    // ÊÇíãÇæÊ ÊŞÑíÈí ÈÑ ÍÓÈ ÍáŞå æ delay
//    while (elapsed < timeout_ms && i < buffer_size - 1) {
//        // ÇÑ ÏÇÏåÇí ÏÑ UART ÂãÏå ÈÇÔÏ
//        while (rx_counter0 > 0 && i < buffer_size - 1) {
//            char c = getchar();  // ÎæÇäÏä í˜ ˜ÇÑÇ˜ÊÑ ÇÒ UART
//            buffer[i++] = c;
//            buffer[i] = '\0';
//
//            // äãÇíÔ ÒäÏå Ñæí GLCD
//            glcd_outtextxy(0, 0, buffer);
//        }
//        delay_ms(1);
//        elapsed++;
//    }
//
//    return (i > 0); // 1 ÇÑ ÍÏÇŞá í˜ ˜ÇÑÇ˜ÊÑ ÏÑíÇİÊ ÔÏå ÈÇÔÏ
//}



// ÈåÊÑíä ÊÇÈÚ ÈÑÇí Çíä ˜ÇÑ
unsigned char read_until_keyword_keep_all(char* buffer, int buffer_size, unsigned long timeout_ms, const char* keyword) {
    int i = 0;
    unsigned long elapsed = 0;
    int found = 0;
    int keyword_len = strlen(keyword);

    memset(buffer, 0, buffer_size);

    while (elapsed < timeout_ms && i < buffer_size - 1) {
        while (rx_counter0 > 0 && i < buffer_size - 1) {
            char c = getchar();
            buffer[i++] = c;
            buffer[i] = '\0';

            // ÈÑÑÓí æÌæÏ keyword
            if (!found && i >= keyword_len) {
                if (strstr(buffer, keyword) != NULL) {
                    found = 1;
                    elapsed = 0;   // ÑíÓÊ ˜ÑÏä ÊÇíãÑ ? ÇÌÇÒå ÈÏíã ÇÏÇãå ÌæÇÈ åã ÈíÇÏ
                }
            }
        }

        if (found && elapsed > 100) {  
            // ÍÏæÏ 100ms ÈÚÏ ÇÒ ÏíÏä ˜áíÏ¡ ÈíÑæä ÈÑæ
            break;
        }

        delay_ms(1);
        elapsed++;
    }

    return (i > 0);
}

//// ÊÇÈÚ ÏÑíÇİÊ ãŞÇÏíÑ Ìáæí ÏÓÊæÑÇÊ
//int extract_value_after_keyword(const char* input, const char* keyword, char* out_value, int out_size) {
//    const char* p = strstr(input, keyword);
//    int i = 0;
//    if (p) {
//        p += strlen(keyword);  // ÈÑæ ÈÚÏ ÇÒ ˜áíÏæÇå
//        while (*p == ' ' || *p == '\t') p++;  // ÑÏ ˜ÑÏä İÇÕáååÇ
//
//        // ˜í ˜ÑÏä ãŞÏÇÑ ÊÇ Çæáíä ÌÏÇ˜ääÏå (, íÇ ÇÓíÓ íÇ CRLF)
//        while (*p && *p != ',' && *p != '\r' && *p != '\n' && *p != ' ' && i < out_size - 1) {
//            out_value[i++] = *p++;
//        }
//        out_value[i] = '\0';
//        return 1;  // ãæİŞ
//    }
//    return 0;  // íÏÇ äÔÏ
//}


int extract_field_after_keyword(const char* input, const char* keyword, int field_index, char* out_value, int out_size)
{
    int current_field = 0;
    int i = 0;
    const char* p = strstr(input, keyword);
    
    if (!p) return 0; // ˜áíÏæÇå íÏÇ äÔÏ

    p += strlen(keyword);      // ÈÑæ ÈÚÏ ÇÒ ˜áíÏæÇå

    // ÑÏ ˜ÑÏä İÇÕáååÇ æ ÊÈåÇ ŞÈá ÇÒ Çæáíä İíáÏ
    while (*p == ' ' || *p == '\t') p++;

    while (*p && current_field <= field_index)
    {
        if (current_field == field_index)
        {
            // ˜í ˜ÑÏä ãŞÏÇÑ İÚáí ÊÇ ˜ÇãÇ¡ CR, LF íÇ space
            while (*p && *p != ',' && *p != '\r' && *p != '\n' && i < out_size - 1)
            {
                out_value[i++] = *p++;
            }
            out_value[i] = '\0';
            return 1; // ãæİŞ
        }

        // ÑİÊä Èå ˜ÇãÇí ÈÚÏí æ ÑÏ ˜ÑÏä İÇÕáååÇí ÇÖÇİí
        while (*p && *p != ',') p++;
        if (*p == ',') p++;  // ÑÏ ˜ÑÏä ˜ÇãÇ
        while (*p == ' ' || *p == '\t') p++; // ÑÏ ˜ÑÏä İÇÕáå ÈÚÏ ÇÒ ˜ÇãÇ
        current_field++;
    }

    return 0; // İíáÏ ãæÑÏäÙÑ íÏÇ äÔÏ
}

void buzzer(unsigned long timeout_ms){

    BUZER_PORT |= (1 << BUZER_PIN); 
    delay_ms(timeout_ms); 
    BUZER_PORT &= ~(1 << BUZER_PIN);

}