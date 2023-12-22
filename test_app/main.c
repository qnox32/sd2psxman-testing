#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <loadfile.h>
#include <libpad.h>
#include <debug.h>

#include "sd2psxman_rpc.h"
#include "../sd2psxman_common.h"

#define PORT PORT_MEMCARD1
#define MAX_GAMEID_LEN 251

#define IRX_DEFINE(mod)                     \
    extern unsigned char mod##_irx[];       \
    extern unsigned int size_##mod##_irx

#define IRX_LOAD(mod)                                                           \
    ID = SifExecModuleBuffer(mod##_irx, size_##mod##_irx, 0, NULL, &RET)        \
    if (ID < 0 || RET ==1) {xprintf("id:%d, ret:%d\n")}

#define xprintf(f_, ...)         \
    printf((f_), ##__VA_ARGS__); \
    scr_printf("    ");          \
    scr_printf((f_), ##__VA_ARGS__);

IRX_DEFINE(mcman);
IRX_DEFINE(sd2psxman);
IRX_DEFINE(sio2man);
IRX_DEFINE(padman);

static struct padButtonStatus padStat;
static char padBuf[256] __attribute__((aligned(64)));

// pad values from this poll
// and from the previous 'frame'
static uint32_t rawPadMask = 0x0000;
static uint32_t lastPadMask = 0x0000;

inline void delay(int seconds)
{
    struct timespec tv;
    tv.tv_sec = seconds;
    tv.tv_nsec = 0;
    nanosleep(&tv, NULL);
}

// not vsynced, but somewhere in the ballpack
// of 60fps so we don't overwhelm the pad
inline void delayframe()
{
    struct timespec tv;
    tv.tv_sec = 0;
    tv.tv_nsec = 16000000;
    nanosleep(&tv, NULL);
}

void test_ping(void)
{
    int rv;

    xprintf("Testing: 0x01 - Ping\n");
    rv = sd2psxman_ping(PORT, 0);

    if (rv > -1)
    {
        xprintf("[PASS] proto ver: 0x%x, prod id: 0x%x, rev id: 0x%x\n", rv >> 16, (rv >> 8) & 0xff, rv & 0xff);
    }
    else
    {
        xprintf("[Fail] error: %i\n", rv);
    }

    xprintf("\n");
}

void test_get_status(void)
{
    xprintf("Not implemented\n");
}

void test_get_card(void)
{
    int rv;

    xprintf("Testing: 0x3 - Get Card\n");
    rv = sd2psxman_get_card(PORT, 0);

    if (rv > -1)
    {
        xprintf("[PASS] current card: %i\n", rv);
    }
    else
    {
        xprintf("[FAIL] error: %i\n", rv);
    }

    xprintf("\n");
}

void test_set_card_next(void)
{
    int rv;
    int temp = -1;

    xprintf("Testing: 0x4 - Set Card [NEXT]\n");
    rv = sd2psxman_get_card(PORT, 0);

    if (rv > -1) {
        sd2psxman_set_card(PORT, 0, SD2PSXMAN_MODE_NEXT, 0);
        delay(7); //give card time to switch
        
        temp = sd2psxman_get_card(PORT, 0);

        if (temp > -1 && temp != rv) {
            xprintf("[PASS] card: %i -> %i\n", rv, temp);
            xprintf("\n");
            return;
        }
    }
    xprintf("[FAIL] %i %i\n", rv, temp);
    xprintf("\n");
}

void test_set_card_prev(void)
{
    int rv;
    int temp = -1;

    xprintf("Testing: 0x4 - Set Card [PREV]\n");
    rv = sd2psxman_get_card(PORT, 0);

    if (rv > -1) {
        sd2psxman_set_card(PORT, 0, SD2PSXMAN_MODE_PREV, 0);
        delay(7); //give card time to switch
        temp = sd2psxman_get_card(PORT, 0);

        if (temp > -1 && temp != rv) {
            xprintf("[PASS] card: %i -> %i\n", rv, temp);
            xprintf("\n");
            return;
        }
    }
    xprintf("[FAIL] %i %i\n", rv, temp);
    xprintf("\n");
}

void test_set_card_num(int num)
{
    int rv;
    int temp = -1;

    xprintf("Testing: 0x4 - Set Card [NUM] (%i)\n", num);
    rv = sd2psxman_get_card(PORT, 0);

    if (rv > -1) {
        sd2psxman_set_card(PORT, 0, SD2PSXMAN_MODE_NUM, num);
        delay(7); //give card time to switch
        temp = sd2psxman_get_card(PORT, 0);

        if (temp > -1 && temp != rv) {
            xprintf("[PASS] card: %i -> %i\n", rv, temp);
            xprintf("\n");
            return;
        }
    }
    xprintf("[FAIL] %i %i\n", rv, temp);
    xprintf("\n");
}

void test_get_channel(void)
{
    int rv;

    xprintf("Testing: 0x5 - Get Channel\n");
    rv = sd2psxman_get_channel(PORT, 0);

    if (rv > -1)
    {
        xprintf("[PASS] current channel: %i\n", rv);
    }
    else
    {
        xprintf("[FAIL] error: %i\n", rv);
    }

    xprintf("\n");
}

void test_set_channel_next(void)
{
    int rv;
    int temp = -1;

    xprintf("Testing: 0x6 - Set Channel [NEXT]\n");
    rv = sd2psxman_get_channel(PORT, 0);

    if (rv > -1) {
        sd2psxman_set_channel(PORT, 0, SD2PSXMAN_MODE_NEXT, 0);
        delay(7); //give card time to switch
        temp = sd2psxman_get_channel(PORT, 0);

        if (temp > -1 && temp != rv) {
            xprintf("[PASS] card: %i -> %i\n", rv, temp);
            xprintf("\n");
            return;
        }
    }
    xprintf("[FAIL] %i %i\n", rv, temp);
    xprintf("\n");
}

void test_set_channel_prev(void)
{
    int rv;
    int temp = -1;

    xprintf("Testing: 0x6 - Set Channel [PREV]\n");
    rv = sd2psxman_get_channel(PORT, 0);
    
    if (rv > -1) {
        sd2psxman_set_channel(PORT, 0, SD2PSXMAN_MODE_PREV, 0);
        delay(7); //give card time to switch        
        temp = sd2psxman_get_channel(PORT, 0);

        if (temp > -1 && temp != rv) {
            xprintf("[PASS] card: %i -> %i\n", rv, temp);
            xprintf("\n");
            return;
        }
    }
    xprintf("[FAIL] %i %i\n", rv, temp);
    xprintf("\n");
}

void test_set_channel_num(int num)
{
    int rv;
    int temp = -1;

    xprintf("Testing: 0x6 - Set Channel [NUM] (%i)\n", num);
    rv = sd2psxman_get_channel(PORT, 0);

    if (rv > -1) {
        sd2psxman_set_channel(PORT, 0, SD2PSXMAN_MODE_NUM, num);
        delay(7); //give card time to switch
        temp = sd2psxman_get_channel(PORT, 0);

        if (temp > -1 && temp != rv) {
            xprintf("[PASS] card: %i -> %i\n", rv, temp);
            xprintf("\n");
            return;
        }
    }
    xprintf("[FAIL] %i %i\n", rv, temp);
    xprintf("\n");
}

void test_get_gameid(void)
{
    int rv;
    char gameid[MAX_GAMEID_LEN];

    xprintf("Testing: 0x7 - Get GameID\n");
    rv = sd2psxman_get_gameid(PORT, 0, gameid);

    if (rv > -1)
    {
        xprintf("[PASS] gameid: %s\n", gameid);
    }
    else
    {
        xprintf("[FAIL] error: %i\n", rv);
    }

    xprintf("\n");
}

void test_set_gameid(char *gameid)
{
    int rv;
    char old_gameid[MAX_GAMEID_LEN];
    char new_gameid[MAX_GAMEID_LEN];

    xprintf("Testing: 0x8 - Set GameID (%s)\n", gameid);
    rv = sd2psxman_get_gameid(PORT, 0, old_gameid);
    
    if (rv > -1) {
        sd2psxman_set_gameid(PORT, 0, gameid);
        delay(1);
        rv = sd2psxman_get_gameid(PORT, 0, new_gameid);

        if (rv > -1) {
            if (strcmp(old_gameid, new_gameid) != 0){
                xprintf("[PASS] gameid: %s -> %s\n", old_gameid, new_gameid);
                return;
            }
        }
    }
    xprintf("[FAIL] %i\n", rv);
    xprintf("\n");
}

void test_unmount_bootcard(void)
{
    int rv;
    int temp;

    xprintf("Testing: 0x30 - Unmount Bootcard\n");
    rv = sd2psxman_get_card(PORT, 0);
    if (rv == 0) {
        sd2psxman_unmount_bootcard(PORT, 0);
        delay(7); //give card time to switch
        temp = sd2psxman_get_card(PORT, 0);

        if (temp > -1 && temp != rv) {
            xprintf("[PASS] bootcard unmounted: %i -> %i\n", rv, temp);
            xprintf("\n");
        } else {
            xprintf("[FAIL] %i %i\n", rv, temp);
            xprintf("\n");
        }
        return;
    }
    xprintf("[FAIL] bootcard not mounted\n");
    xprintf("\n");
}

/// @brief Read pad vals and update prev to determine new button presses/releases
/// @returns successful read
bool update_pad()
{

    int port = 0;
    int slot = 0;

    lastPadMask = rawPadMask;

    int rv = padRead(port, slot, &padStat);

    if (rv == 0)
    {
        xprintf("padRead failed: %d\n", rv);
        rawPadMask = 0x0000;
        lastPadMask = 0x0000;
        return false;
    }

    rawPadMask = 0xFFFF ^ padStat.btns;
    
    return true;
}

bool released(int button)
{
    return ((lastPadMask & button) == button) && ((rawPadMask & button) != button);
}

bool all_released()
{
    return rawPadMask == 0x0000 && lastPadMask != 0x0000;
}

bool init_pad()
{
    // First pad only
    int port = 0;
    int slot = 0;

    int rv = 0;

    rv = padInit(port);
    if (rv != 1)
    {
        xprintf("padInit failed: %d\n", rv);
        return false;
    }

    rv = padPortOpen(port, slot, &padBuf);
    if (rv == 0)
    {
        xprintf("padPortOpen failed: %d\n", rv);
        return false;
    }

    // Wait for the pad to become ready
    rv = padGetState(port, slot);
    while ((rv != PAD_STATE_STABLE) && (rv != PAD_STATE_FINDCTP1))
    {
        if (rv == PAD_STATE_DISCONN)
        {
            xprintf("Pad port%d/slot%d) is disconnected\n", port, slot);
            return false;
        }
        rv = padGetState(port, slot);
    }

    // not necessary, but the LED is a useful indicator
    padSetMainMode(port, slot, PAD_MMODE_DUALSHOCK, PAD_MMODE_LOCK);

    // It doesn't like being rushed.
    for (int i = 0; i < 30; i++)
    {
        delayframe();
    }

    // Pre-read pad vals
    update_pad();

    xprintf("Pad %d ready in slot %d\n", port, slot);

    return true;
}

void print_menu_header()
{
    xprintf(" \n");
    xprintf("--------------------\n");
    xprintf("><  = Ping\n");
    xprintf("/\\  = Get GameID\n");
    xprintf("[]  = Set Game = Katamari (SLUS-21230) \n");
    xprintf("()  = Set Game = GT3 (PBPX-95503)\n");
    xprintf(" \n");
    xprintf("->  = Get Card\n");
    xprintf("--  = Get Status\n");
    xprintf(" \n");
    xprintf("R1 = Next Chan\n");
    xprintf("L1 = Prev Chan\n");
    xprintf("R2 = Next Card\n");
    xprintf("L2 = Prev Card\n");
    xprintf("R3 = Exit\n");
    xprintf("--------------------\n");
}

void menu_loop()
{

    while (true)
    {

        scr_clear();
        print_menu_header();

        update_pad();

        // Prevent spamming the sio every frame 
        // as if we were printing to the screen.
        while (!all_released())
        {
            delayframe();
            update_pad();
        }

        xprintf("Processing...\n");

        // Ping & GameID

        if (released(PAD_CROSS))
        {
            delay(1);
            test_ping();
        }
        else if (released(PAD_CIRCLE))
        {
            // Gran Turismo 3 - A-Spec
            char gameid[0x250] = {'S', 'L', 'U', 'S', '-', '2', '1', '2', '3', '0', '\0'};
            test_set_gameid(gameid);
        }
        else if (released(PAD_SQUARE))
        {
            // Gran Turismo 3 - A-Spec
            char gameid[0x250] = {'P', 'B', 'P', 'X', '-', '9', '5', '5', '0', '3', '\0'};
            test_set_gameid(gameid);
        }
        else if (released(PAD_TRIANGLE))
        {
            xprintf("Get Game ID not implemented\n");
        }

        // Chan and Card Switch:

        if (released(PAD_L1))
        {
            test_set_channel_prev();
        }
        else if (released(PAD_L2))
        {
            test_set_card_prev();
        }
        else if (released(PAD_R1))
        {
            test_set_channel_next();
        }
        else if (released(PAD_R2))
        {
            test_set_card_next();
        }

        // Read info:

        if (released(PAD_START))
        {
            test_get_card();
        }
        else if (released(PAD_SELECT))
        {
            test_get_status();
        }
        else if (released(PAD_R3))
        {
            break;
        }


        xprintf("Push to continue... \n" );
        update_pad();
        while (!all_released())
        {
            delayframe();
            update_pad();
        }

        xprintf("Done...\n");
        
    } // while (true);
}

void auto_test()
{

    xprintf("Performing auto test sequence...\n");

    time_t t;
    srand((unsigned)time(&t));

    char gameid[0x250] = {'S', 'L', 'U', 'S', '-', '2', '0', '9', '1', '5', '\0'};

    delay(2);
    test_ping();
    delay(2);
    test_get_card();
    delay(2);
    test_set_card_next();
    delay(2);
    test_set_card_prev();
    delay(2);
    test_set_card_num(rand() % 1024);
    delay(2);
    test_get_channel();
    delay(2);
    test_set_channel_next();
    delay(2);
    test_set_channel_prev();
    delay(2);
    test_set_channel_num((rand() % 7) + 1);
    delay(2);
    test_get_gameid();
    delay(2);
    test_set_gameid(gameid);

    xprintf("Auto test complete\n");
}

/// @return a button was pressed on the pad, interrupting the countdown.
bool countdown()
{

    xprintf("Auto test will run in 3 seconds, press the Any key for manual operation\n");

    for (int i = 4; i > 0; i--)
    {
        xprintf("%d...\n", i-1);
        for (int j = 0; j < 60; j++)
        {
            delayframe();
            update_pad();
            if (all_released())
            {
                return true;
            }
        }
    }
    return false;
}

int main()
{
    int ID, RET;
    init_scr();

    xprintf("Loading sio2man:");
    IRX_LOAD(sio2man);
    delay(1);

    xprintf("Loading sd2psxman:");
    IRX_LOAD(sd2psxman);
    delay(1);
    
    xprintf("Loading mcman:");
    IRX_LOAD(mcman);

    xprintf("Loading padman:");
    IRX_LOAD(padman);

    sd2psxman_init();

    // Perform a short automatic sequence
    // if we fail to init the pads or
    // interrupt the countdown timer

    bool padInited = init_pad();

    if (!padInited)
    {
        xprintf("Pad init failed...\n");
        auto_test();
        return 0;
    }

    bool interrupted = countdown();

    if (!interrupted)
    {
        xprintf("No button press, running auto test...\n");
        auto_test();
        return 0;
    }

    // in case you've been mashing the keys a bit
    update_pad();
    delay(1);

    menu_loop();

    return 0;
}
