#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <loadfile.h>
#include <libpad.h>


#include "sd2psxman_rpc.h"
#include "../sd2psxman_common.h"

#define PORT PORT_MEMCARD1
#define MAX_GAMEID_LEN 251

#define IRX_DEFINE(mod)                     \
    extern unsigned char mod##_irx[];       \
    extern unsigned int size_##mod##_irx

#define IRX_LOAD(mod)                                                           \
    if (SifExecModuleBuffer(mod##_irx, size_##mod##_irx, 0, NULL, NULL) < 0)    \
    printf("Could not load ##mod##\n")

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

    printf("Testing: 0x01 - Ping\n");
    rv = sd2psxman_ping(PORT, 0);
    
    if (rv > -1)
        printf("[PASS] proto ver: 0x%x, prod id: 0x%x, rev id: 0x%x\n", rv >> 16, (rv >> 8) & 0xff, rv & 0xff);
    else
        printf("[Fail] error: %i\n", rv);

    printf("\n");
}

void test_get_status(void)
{
    printf("Not implemented\n");
}

void test_get_card(void)
{
    int rv;

    printf("Testing: 0x3 - Get Card\n");
    rv = sd2psxman_get_card(PORT, 0);

    if (rv > -1)
        printf("[PASS] current card: %i\n", rv);
    else
        printf("[FAIL] error: %i\n", rv);

    printf("\n");
}

void test_set_card_next(void)
{
    int rv;
    int temp = -1;

    printf("Testing: 0x4 - Set Card [NEXT]\n");
    rv = sd2psxman_get_card(PORT, 0);

    if (rv > -1) {
        sd2psxman_set_card(PORT, 0, SD2PSXMAN_MODE_NEXT, 0);
        delay(7); //give card time to switch
        
        temp = sd2psxman_get_card(PORT, 0);

        if (temp > -1 && temp != rv) {
            printf("[PASS] card: %i -> %i\n", rv, temp);
            printf("\n");
            return;
        }
    }
    printf("[FAIL] %i %i\n", rv, temp);
    printf("\n");
}

void test_set_card_prev(void)
{
    int rv;
    int temp = -1;

    printf("Testing: 0x4 - Set Card [PREV]\n");
    rv = sd2psxman_get_card(PORT, 0);

    if (rv > -1) {
        sd2psxman_set_card(PORT, 0, SD2PSXMAN_MODE_PREV, 0);
        delay(7); //give card time to switch
        temp = sd2psxman_get_card(PORT, 0);

        if (temp > -1 && temp != rv) {
            printf("[PASS] card: %i -> %i\n", rv, temp);
            printf("\n");
            return;
        }
    }
    printf("[FAIL] %i %i\n", rv, temp);
    printf("\n");
}

void test_set_card_num(int num)
{
    int rv;
    int temp = -1;

    printf("Testing: 0x4 - Set Card [NUM] (%i)\n", num);
    rv = sd2psxman_get_card(PORT, 0);

    if (rv > -1) {
        sd2psxman_set_card(PORT, 0, SD2PSXMAN_MODE_NUM, num);
        delay(7); //give card time to switch
        temp = sd2psxman_get_card(PORT, 0);

        if (temp > -1 && temp != rv) {
            printf("[PASS] card: %i -> %i\n", rv, temp);
            printf("\n");
            return;
        }
    }
    printf("[FAIL] %i %i\n", rv, temp);
    printf("\n");
}

void test_get_channel(void)
{
    int rv;

    printf("Testing: 0x5 - Get Channel\n");
    rv = sd2psxman_get_channel(PORT, 0);

    if (rv > -1)
        printf("[PASS] current channel: %i\n", rv);
    else
        printf("[FAIL] error: %i\n", rv);

    printf("\n");
}

void test_set_channel_next(void)
{
    int rv;
    int temp = -1;

    printf("Testing: 0x6 - Set Channel [NEXT]\n");
    rv = sd2psxman_get_channel(PORT, 0);

    if (rv > -1) {
        sd2psxman_set_channel(PORT, 0, SD2PSXMAN_MODE_NEXT, 0);
        delay(7); //give card time to switch
        temp = sd2psxman_get_channel(PORT, 0);

        if (temp > -1 && temp != rv) {
            printf("[PASS] card: %i -> %i\n", rv, temp);
            printf("\n");
            return;
        }
    }
    printf("[FAIL] %i %i\n", rv, temp);
    printf("\n");
}

void test_set_channel_prev(void)
{
    int rv;
    int temp = -1;

    printf("Testing: 0x6 - Set Channel [PREV]\n");
    rv = sd2psxman_get_channel(PORT, 0);
    
    if (rv > -1) {
        sd2psxman_set_channel(PORT, 0, SD2PSXMAN_MODE_PREV, 0);
        delay(7); //give card time to switch        
        temp = sd2psxman_get_channel(PORT, 0);

        if (temp > -1 && temp != rv) {
            printf("[PASS] card: %i -> %i\n", rv, temp);
            printf("\n");
            return;
        }
    }
    printf("[FAIL] %i %i\n", rv, temp);
    printf("\n");
}

void test_set_channel_num(int num)
{
    int rv;
    int temp = -1;

    printf("Testing: 0x6 - Set Channel [NUM] (%i)\n", num);
    rv = sd2psxman_get_channel(PORT, 0);

    if (rv > -1) {
        sd2psxman_set_channel(PORT, 0, SD2PSXMAN_MODE_NUM, num);
        delay(7); //give card time to switch
        temp = sd2psxman_get_channel(PORT, 0);

        if (temp > -1 && temp != rv) {
            printf("[PASS] card: %i -> %i\n", rv, temp);
            printf("\n");
            return;
        }
    }
    printf("[FAIL] %i %i\n", rv, temp);
    printf("\n");
}

void test_get_gameid(void)
{
    int rv;
    char gameid[MAX_GAMEID_LEN];

    printf("Testing: 0x7 - Get GameID\n");
    rv = sd2psxman_get_gameid(PORT, 0, gameid);
    
    if (rv > -1)
        printf("[PASS] gameid: %s\n", gameid);
    else
        printf("[FAIL] error: %i\n", rv);

    printf("\n");
}

void test_set_gameid(char *gameid)
{
    int rv;
    char old_gameid[MAX_GAMEID_LEN];
    char new_gameid[MAX_GAMEID_LEN];

    printf("Testing: 0x8 - Set GameID (%s)\n", gameid);
    rv = sd2psxman_get_gameid(PORT, 0, old_gameid);
    
    if (rv > -1) {
        sd2psxman_set_gameid(PORT, 0, gameid);
        delay(1);
        rv = sd2psxman_get_gameid(PORT, 0, new_gameid);

        if (rv > -1) {
            if (strcmp(old_gameid, new_gameid) != 0){
                printf("[PASS] gameid: %s -> %s\n", old_gameid, new_gameid);
                return;
            }
        }
    }
    printf("[FAIL] %i\n", rv);
    printf("\n");
}

void test_unmount_bootcard(void)
{
    int rv;
    int temp;

    printf("Testing: 0x30 - Unmount Bootcard\n");
    rv = sd2psxman_get_card(PORT, 0);
    if (rv == 0) {
        sd2psxman_unmount_bootcard(PORT, 0);
        delay(7); //give card time to switch
        temp = sd2psxman_get_card(PORT, 0);

        if (temp > -1 && temp != rv) {
            printf("[PASS] bootcard unmounted: %i -> %i\n", rv, temp);
            printf("\n");
        } else {
            printf("[FAIL] %i %i\n", rv, temp);
            printf("\n");
        }
        return;
    }
    printf("[FAIL] bootcard not mounted\n");
    printf("\n");
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
        printf("padRead failed: %d\n", rv);
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
        printf("padInit failed: %d\n", rv);
        return false;
    }

    rv = padPortOpen(port, slot, &padBuf);
    if (rv == 0)
    {
        printf("padPortOpen failed: %d\n", rv);
        return false;
    }

    // Wait for the pad to become ready
    rv = padGetState(port, slot);
    while ((rv != PAD_STATE_STABLE) && (rv != PAD_STATE_FINDCTP1))
    {
        if (rv == PAD_STATE_DISCONN)
        {
            printf("Pad port%d/slot%d) is disconnected\n", port, slot);
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

    printf("Pad %d ready in slot %d\n", port, slot);

    return true;
}

void print_menu_header()
{
    printf("\n");
    printf("--------------------\n");
    printf("><  = Ping\n");
    printf("/\\  = Get GameID\n");
    printf("[]  = Set Game = Katamari (SLUS-21230) \n");
    printf("()  = Set Game = GT3 (PBPX-95503)\n");
    printf("\n");
    printf("->  = Get Card\n");
    printf("--  = Get Status\n");
    printf("\n");
    printf("R1 = Next Chan\n");
    printf("L1 = Prev Chan\n");
    printf("R2 = Next Card\n");
    printf("L2 = Prev Card\n");
    printf("R3 = Exit\n");
    printf("--------------------\n");
}

void menu_loop()
{

    while (true)
    {

        print_menu_header();

        update_pad();

        // Prevent spamming the sio every frame 
        // as if we were printing to the screen.
        while (!all_released())
        {
            delayframe();
            update_pad();
        }

        printf("Processing...\n");

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
            printf("Triangle pressed\n");
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

        printf("Done...\n");
        
    } // while (true);
}

void auto_test()
{

    printf("Performing auto test sequence...\n");

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

    printf("Auto test complete\n");
}

/// @return a button was pressed on the pad, interrupting the countdown.
bool countdown()
{

    printf("Auto test will run in 3 seconds, press the Any key for manual operation\n");

    for (int i = 4; i > 0; i--)
    {
        printf("%d... %ld\n", i, rawPadMask);
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

    printf("Loading sio2man\n");
    IRX_LOAD(sio2man);
    delay(1);

    printf("Loading sd2psxman\n");
    IRX_LOAD(sd2psxman);
    delay(1);
    
    printf("Loading mcman\n");
    IRX_LOAD(mcman);

    printf("Loading padman\n");
    IRX_LOAD(padman);

    sd2psxman_init();

    // Perform a short automatic sequence
    // if we fail to init the pads or
    // interrupt the countdown timer

    bool padInited = init_pad();

    if (!padInited)
    {
        printf("Pad init failed...\n");
        auto_test();
        return 0;
    }

    bool interrupted = countdown();

    if (!interrupted)
    {
        printf("No button press, running auto test...\n");
        auto_test();
        return 0;
    }

    menu_loop();

    return 0;
}
