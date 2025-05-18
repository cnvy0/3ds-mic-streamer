#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <malloc.h>
#include <3ds.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SOC_BUFFERSIZE 0x100000

u32 *soc_buffer;

// options
const char *pc_ip = "0.0.0.0";
short unsigned int pc_port = 12334;

struct sockaddr_in pc_addr;

int initSocket()
{
    soc_buffer = (u32 *)memalign(0x1000, SOC_BUFFERSIZE);
    if (!soc_buffer)
        return -1;

    int ret = socInit(soc_buffer, SOC_BUFFERSIZE);
    return ret;
}

void replaceScreenText(const char *text)
{
    printf("\033[2J\033[H");
    printf(text);
}

static SwkbdCallbackResult ServerChangeCallback(void *user, const char **ppMessage, const char *text, size_t textlen)
{
    const char *type_changing = (const char *)user;

    if (strstr(type_changing, "IP"))
    {
        pc_ip = text;
        pc_addr.sin_addr.s_addr = inet_addr(pc_ip);
    }
    else if (strstr(type_changing, "PORT"))
    {
        char *end;
        unsigned long port = strtoul(text, &end, 10);

        if (*end == '\0' && port <= 65535)
        {
            pc_port = (unsigned short)port;
            pc_addr.sin_port = htons(pc_port);
        }
        else
        {
            *ppMessage = "Invalid port number.";
        }
    }

    printf("New server: %s:%hu\n", pc_ip, pc_port);

    return SWKBD_CALLBACK_OK;
}

int main(int argc, char *argv[])
{
    // stuff
    gfxInitDefault();
    consoleInit(GFX_TOP, NULL);

    printf("Current server: %s:%hu\n", pc_ip, pc_port);
    printf("Press Y to change server IP\n");
    printf("Press X to change server port\n");

    // mic
    bool initialized = true;

    u32 micbuf_size = 0x30000;
    u32 micbuf_pos = 0;
    u8 *micbuf = memalign(0x1000, micbuf_size);

    // printf("Initializing CSND...\n");
    if (R_FAILED(csndInit()))
    {
        initialized = false;
        printf("Could not initialize CSND.\n");
    }
    // else
    //     printf("CSND initialized.\n");

    // printf("Initializing MIC...\n");
    if (R_FAILED(micInit(micbuf, micbuf_size)))
    {
        initialized = false;
        printf("Could not initialize MIC.\n");
    }
    // else
    //     printf("MIC initialized.\n");

    u32 micbuf_datasize = micGetSampleDataSize();

    // sockets
    if (initialized)
        printf("Hold A to record, and release to stop.\n");
    printf("Press START to exit.\n");

    if (initSocket() != 0)
    {
        printf("Socket init failed.\n");
        goto exit;
    }

    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock < 0)
    {
        printf("Socket creation failed.\n");
        socExit();
        return -1;
    }

    memset(&pc_addr, 0, sizeof(pc_addr));
    pc_addr.sin_family = AF_INET;
    pc_addr.sin_port = htons(pc_port);
    pc_addr.sin_addr.s_addr = inet_addr(pc_ip);

    // Main loop
    while (aptMainLoop())
    {
        gspWaitForVBlank();
        hidScanInput();

        u32 kDown = hidKeysDown();
        if (kDown & KEY_START)
        {
            break; // break in order to return to hbmenu
        }

        static SwkbdState swkbd;
        static char mybuf[60];
        static SwkbdStatusData swkbdStatus;
        static SwkbdLearningData swkbdLearning;
        // SwkbdButton button = SWKBD_BUTTON_NONE;

        if (initialized)
        {
            if (kDown & KEY_Y)
            {
                swkbdInit(&swkbd, SWKBD_TYPE_NORMAL, 3, -1);
                swkbdSetInitialText(&swkbd, mybuf);
                swkbdSetHintText(&swkbd, "Enter server IP");
                swkbdSetButton(&swkbd, SWKBD_BUTTON_LEFT, "Back", false);
                swkbdSetButton(&swkbd, SWKBD_BUTTON_RIGHT, "Enter", true);
                swkbdSetFilterCallback(&swkbd, ServerChangeCallback, "IP");
                static bool reload = false;
                swkbdSetStatusData(&swkbd, &swkbdStatus, reload, true);
                swkbdSetLearningData(&swkbd, &swkbdLearning, reload, true);
                reload = true;
                //button = 
                swkbdInputText(&swkbd, mybuf, sizeof(mybuf));
            }

            if (kDown & KEY_X)
            {
                swkbdInit(&swkbd, SWKBD_TYPE_NUMPAD, 1, 5);
                swkbdSetValidation(&swkbd, SWKBD_ANYTHING, 0, 0);
                swkbdSetFeatures(&swkbd, SWKBD_FIXED_WIDTH);
                swkbdSetFilterCallback(&swkbd, ServerChangeCallback, "PORT");
                //button = 
                swkbdInputText(&swkbd, mybuf, sizeof(mybuf));
            }

            if (kDown & KEY_A)
            {
                micbuf_pos = 0;

                if (R_SUCCEEDED(MICU_StartSampling(MICU_ENCODING_PCM16_SIGNED, MICU_SAMPLE_RATE_16360, 0, micbuf_datasize, true)))
                    replaceScreenText("Now recording.\n");
                else
                    replaceScreenText("Failed to start sampling.\n");
            }

            if (hidKeysHeld() & KEY_A)
            {
                u32 new_pos = micGetLastSampleOffset();
                if (new_pos != micbuf_pos)
                {
                    if (new_pos > micbuf_pos)
                    {
                        // Send directly
                        sendto(sock, &micbuf[micbuf_pos], new_pos - micbuf_pos, 0, (struct sockaddr *)&pc_addr, sizeof(pc_addr));
                    }
                    else
                    {
                        // Handle wrap-around
                        sendto(sock, &micbuf[micbuf_pos], micbuf_datasize - micbuf_pos, 0, (struct sockaddr *)&pc_addr, sizeof(pc_addr));
                        sendto(sock, micbuf, new_pos, 0, (struct sockaddr *)&pc_addr, sizeof(pc_addr));
                    }
                    micbuf_pos = new_pos;
                }
            }

            if (hidKeysUp() & KEY_A)
            {
                replaceScreenText("Not recording.\n");
                if (R_FAILED(MICU_StopSampling()))
                    replaceScreenText("Failed to stop sampling.\n");
            }
        }

        gfxFlushBuffers();
        gfxSwapBuffers();
    }

exit:
    micExit();
    free(micbuf);
    socExit();
    free(soc_buffer);

    csndExit();
    gfxExit();
    return 0;
}
