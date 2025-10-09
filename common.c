#include "common.h"
#include <string.h>
#include <delay.h>
#include <mega64a.h>
#include <glcd.h>

#define glcd_pixel(x, y, color) glcd_setpixel(x, y)
#define read_flash_byte(p) (*(p))

// --- «—”«· œ” Ê— AT ---
void send_at_command(char *command)
{
    printf("%s\r\n", command);
}

// --- Å«ò ò—œ‰ »«›— USART0 ---
void uart_flush0(void)
{
    unsigned char dummy;

    // Å«ò ò—œ‰ —ÃÌ” — ”Œ ù«›“«—Ì
    while (UCSR0A & (1<<RXC0)) {
        dummy = UDR0;
    }
//
//    // Å«ò ò—œ‰ »«›— ‰—„ù«›“«—Ì
//    rx_wr_index0 = rx_rd_index0 = 0;
//    rx_counter0 = 0;
//    rx_buffer_overflow0 = 0;
}


void uart_buffer_reset(void) {
    rx_wr_index0 = rx_rd_index0 = 0;
    rx_counter0 = 0;
    rx_buffer_overflow0 = 0;
}


//unsigned char read_serial_timeout_simple(char* buffer, int buffer_size, unsigned long timeout_ms) {
//    int i = 0;
//    unsigned long elapsed = 0;
//
//    // »«›— „Õ·Ì —« ’›— „Ìùò‰Ì„
//    memset(buffer, 0, buffer_size);
//
//    //  «Ì„ù«Ê   ﬁ—Ì»Ì »— Õ”» Õ·ﬁÂ Ê delay
//    while (elapsed < timeout_ms && i < buffer_size - 1) {
//        // «ê— œ«œÂù«Ì œ— UART ¬„œÂ »«‘œ
//        while (rx_counter0 > 0 && i < buffer_size - 1) {
//            char c = getchar();  // ŒÊ«‰œ‰ Ìò ò«—«ò — «“ UART
//            buffer[i++] = c;
//            buffer[i] = '\0';
//
//            // ‰„«Ì‘ “‰œÂ —ÊÌ GLCD
//            glcd_outtextxy(0, 0, buffer);
//        }
//        delay_ms(1);
//        elapsed++;
//    }
//
//    return (i > 0); // 1 «ê— Õœ«ﬁ· Ìò ò«—«ò — œ—Ì«›  ‘œÂ »«‘œ
//}



// »Â —Ì‰  «»⁄ »—«Ì «Ì‰ ò«—
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

            // »——”Ì ÊÃÊœ keyword
            if (!found && i >= keyword_len) {
                if (strstr(buffer, keyword) != NULL) {
                    found = 1;
                    elapsed = 0;   // —Ì”  ò—œ‰  «Ì„— ? «Ã«“Â »œÌ„ «œ«„Â ÃÊ«» Â„ »Ì«œ
                }
            }
        }

        if (found && elapsed > 100) {  
            // ÕœÊœ 100ms »⁄œ «“ œÌœ‰ ò·Ìœ° »Ì—Ê‰ »—Ê
            break;
        }

        delay_ms(1);
        elapsed++;
    }

    return (i > 0);
}

////  «»⁄ œ—Ì«›  „ﬁ«œÌ— Ã·ÊÌ œ” Ê—« 
//int extract_value_after_keyword(const char* input, const char* keyword, char* out_value, int out_size) {
//    const char* p = strstr(input, keyword);
//    int i = 0;
//    if (p) {
//        p += strlen(keyword);  // »—Ê »⁄œ «“ ò·ÌœÊ«éÂ
//        while (*p == ' ' || *p == '\t') p++;  // —œ ò—œ‰ ›«’·ÂùÂ«
//
//        // òÅÌ ò—œ‰ „ﬁœ«—  « «Ê·Ì‰ Ãœ«ò‰‰œÂ (, Ì« «”ÅÌ” Ì« CRLF)
//        while (*p && *p != ',' && *p != '\r' && *p != '\n' && *p != ' ' && i < out_size - 1) {
//            out_value[i++] = *p++;
//        }
//        out_value[i] = '\0';
//        return 1;  // „Ê›ﬁ
//    }
//    return 0;  // ÅÌœ« ‰‘œ
//}


int extract_field_after_keyword(const char* input, const char* keyword, int field_index, char* out_value, int out_size)
{
    int current_field = 0;
    int i = 0;
    const char* p = strstr(input, keyword);
    
    if (!p) return 0; // ò·ÌœÊ«éÂ ÅÌœ« ‰‘œ

    p += strlen(keyword);      // »—Ê »⁄œ «“ ò·ÌœÊ«éÂ

    // —œ ò—œ‰ ›«’·ÂùÂ« Ê  »ùÂ« ﬁ»· «“ «Ê·Ì‰ ›Ì·œ
    while (*p == ' ' || *p == '\t') p++;

    while (*p && current_field <= field_index)
    {
        if (current_field == field_index)
        {
            // òÅÌ ò—œ‰ „ﬁœ«— ›⁄·Ì  « ò«„«° CR, LF Ì« space
            while (*p && *p != ',' && *p != '\r' && *p != '\n' && i < out_size - 1)
            {
                out_value[i++] = *p++;
            }
            out_value[i] = '\0';
            return 1; // „Ê›ﬁ
        }

        // —› ‰ »Â ò«„«Ì »⁄œÌ Ê —œ ò—œ‰ ›«’·ÂùÂ«Ì «÷«›Ì
        while (*p && *p != ',') p++;
        if (*p == ',') p++;  // —œ ò—œ‰ ò«„«
        while (*p == ' ' || *p == '\t') p++; // —œ ò—œ‰ ›«’·Â »⁄œ «“ ò«„«
        current_field++;
    }

    return 0; // ›Ì·œ „Ê—œ‰Ÿ— ÅÌœ« ‰‘œ
}


//
//int extract_field_after_keyword(const char* input, const char* keyword, int field_index, char* out_value, int out_size)
//{
//    int current_field = 0;
//    int i = 0;
//    const char* p = strstr(input, keyword);
//    
//    if (!p) return 0; // ò·ÌœÊ«éÂ ÅÌœ« ‰‘œ
//
//    p += strlen(keyword);      // »—Ê »⁄œ «“ ò·ÌœÊ«éÂ
//    while (*p == ' ' || *p == '\t') p++; // —œ ò—œ‰ ›«’·ÂùÂ«
//
//    while (*p && current_field <= field_index)
//    {
//        if (current_field == field_index)
//        {
//            // òÅÌ ò—œ‰ „ﬁœ«— ›⁄·Ì  « ò«„« Ì« CRLF Ì« «”ÅÌ”
//            while (*p && *p != ',' && *p != '\r' && *p != '\n' && i < out_size - 1)
//            {
//                out_value[i++] = *p++;
//            }
//            out_value[i] = '\0';
//            return 1; // „Ê›ﬁ
//        }
//
//        // —› ‰ »Â ò«„«Ì »⁄œÌ
//        while (*p && *p != ',') p++;
//        if (*p == ',') p++; // —œ ò—œ‰ ò«„«
//        current_field++;
//    }
//
//    return 0; // ›Ì·œ „Ê—œ‰Ÿ— ÅÌœ« ‰‘œ
//}



//unsigned char read_serial_response(char* buffer, int buffer_size, int timeout_ms, const char* end_pattern) {
//    int i = 0;
//    unsigned int elapsed = 0;
//
//    memset(buffer, 0, buffer_size);
//
//    while (elapsed < (unsigned)timeout_ms) {
//        while (rx_counter0 > 0 && i < buffer_size - 1) {
//            buffer[i++] = getchar();
//        }
//
//        // »——”Ì «·êÊÌ Å«Ì«‰ œ«œÂ
//        if (end_pattern != NULL && strstr(buffer, end_pattern)) {
//            buffer[i] = '\0';
//            return 1;
//        }
//
//        if (i >= buffer_size - 1) {
//            buffer[i] = '\0';
//            return 1;
//        }
//
//        delay_ms(1);
//        elapsed++;
//    }
//
//    buffer[i] = '\0';
//    return (i > 0);
//}


//// --- ŒÊ«‰œ‰ Å«”Œ ”—Ì«· »« timeout ---
//unsigned char read_serial_response(char* buffer, int buffer_size, int timeout_ms, const char* expected_response) {
//    int i = 0;
//    unsigned int elapsed = 0;
//
//    // ?C? ??I? EC?? ?C?E?
//    memset(buffer, 0, buffer_size);
//
//    while (elapsed < (unsigned)timeout_ms) {
//        // ?? EC?? E?C? EC?E??C? I? I?E?? ????? UART ?C EI?C??I
//        while (rx_counter0 > 0 && i < buffer_size - 1) {
//            buffer[i++] = getchar();  // getchar C? ????? EC?? ???A?I
//        }
//        // C?? C?EUC??C? ?C I?I??? ??I E???I??
//        if (strstr(buffer, expected_response)) {
//            return 1;
//        }
//        delay_ms(1);
//        elapsed++;
//    }
//    // ?? C? ?C?C? EC???C?E ?? ??EC? I??? ?? ???????
//    return (strstr(buffer, expected_response) != NULL);
//}


