#include "keypad.h"
#include "common.h"
#include <glcd.h>
#include <font5x7.h>

#define MAX_INPUT_LEN 20
#define MAX_TOTAL_LEN 20
#define POLL_DELAY_MS 20
#define s 3



char get_key(void)
{
    unsigned char row, col;
    const unsigned char column_pins[3] = {COL1_PIN, COL2_PIN, COL3_PIN};
    const unsigned char row_pins[4] = {ROW1_PIN, ROW2_PIN, ROW3_PIN, ROW4_PIN};

//    const char key_map[4][3] = {
//        {'1', '2', '3'},
//        {'4', '5', '6'},
//        {'7', '8', '9'},
//        {'*', '0', '#'}
//    };

    const char key_map[4][3] = {
        {'*', '0', '#'},  // —œÌ› 1
        {'9', '8', '7'},  // —œÌ› 2
        {'6', '5', '4'},  // —œÌ› 3
        {'3', '2', '1'}   // —œÌ› 4
    };

    for (col = 0; col < 3; col++)
    {
        KEYPAD_PORT |= (1 << COL1_PIN) | (1 << COL2_PIN) | (1 << COL3_PIN);
        KEYPAD_PORT &= ~(1 << column_pins[col]);

        for (row = 0; row < 4; row++)
        {
            if (!(KEYPAD_PIN & (1 << row_pins[row])))
            {
                delay_ms(10);
                if (!(KEYPAD_PIN & (1 << row_pins[row])))
                {
                    while (!(KEYPAD_PIN & (1 << row_pins[row])));
                    return key_map[row][col];
                }
            }
        }
    }

    return 0;
}


//
void test_keypad(void)
{
    char key;
    char buf[2];


    glcd_clear();
    glcd_outtextxy(0, 0, "Press a key...");

    while (1)
    {
        key = get_key();  //  «»⁄Ì òÂ òÌùÅœ —Ê „ÌùŒÊ‰Â
        if (key != 0)
        {
            glcd_clear();
            glcd_outtextxy(0, 0, "Key Pressed:");

            buf[0] = key;  // ò«—«ò — ò·Ìœ
            buf[1] = '\0';

            glcd_outtextxy(0, 16, buf);  // ‰„«Ì‘ —ÊÌ GLCD
            delay_ms(200);               // ò„Ì „òÀ »—«Ì œÌœ‰
        }
    }
}



//  «»⁄ ò„òÌ: »——”Ì „Ìùò‰œ ¬Œ—Ì‰ ”Â ò«—«ò — ›ﬁÿ '#' Â” ‰œ
//uint8_t last_three_hash_only(char *buf, unsigned int len)
//{
//    if (len < 3) return 0;
//    return (buf[len-1] == '#' && buf[len-2] == '#' && buf[len-3] == '#');
//}


unsigned char get_number_from_keypad(char *buffer_key)
{
    unsigned int i = 0;
    unsigned char sep_count = 0;
    char key;

    glcd_clear();
    glcd_outtextxy(0, 0, "Operator MODE");
    glcd_outtextxy(0, 12, "(# cl - *** can)");
    glcd_outtextxy(0, 30, ""); // Œÿ »⁄œÌ »—«Ì ⁄œœÂ«

    buffer_key[0] = '\0';

    while (1)
    {
        key = get_key();
        if (!key)
            continue;

        // Å«ò ò—œ‰ »«›— »« *
        if (key == '#')
        {
            i = 0;
            sep_count = 0;
            buffer_key[0] = '\0';
            glcd_clear();
            glcd_outtextxy(0, 0, "Operator MODE");
            glcd_outtextxy(0, 12, "(# cl- *** can)");
            glcd_outtextxy(0, 30, "");
            continue;
        }

        // «⁄œ«œ
        if (key >= '0' && key <= '9')
        {
            if (i < (MAX_TOTAL_LEN - 2))
            {
                buffer_key[i++] = key;
                buffer_key[i] = '\0';
                glcd_outtextxy(0, 30, buffer_key);
            }
            continue;
        }

        // Ãœ«ò‰‰œÂ #
        if (key == '*')
        {
            if (i == 0) continue; // —‘ Â Œ«·Ì° ò«—Ì ‰ò‰

            if (i < (MAX_TOTAL_LEN - 2))
            {
                buffer_key[i++] = '*';
                buffer_key[i] = '\0';
                sep_count++;
                glcd_outtextxy(0, 30, buffer_key);

                // «ê— ”Â # Å‘  ”— Â„ »œÊ‰ ⁄œœ »Ì‰‘«‰ ? ·€Ê
                if (i >= 3 && buffer_key[i-1] == '*' && buffer_key[i-2] == '*' && buffer_key[i-3] == '*')
                {
                    buffer_key[0] = '\0'; // Å«ò ò—œ‰ »«›—
                    glcd_clear();
                    glcd_outtextxy(0, 0, "Canceled!");
                    delay_ms(250);
                    return 0;
                }

                // «ê— ”Â # »« ⁄œœ »Ì‰‘«‰ ? œ«œÂ ò«„·
                if (sep_count >= 3)
                    return 1;
            }
            continue;
        }
    }
}
