#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <inttypes.h>
#include <stdbool.h>
#include <io.h>
#include <windows.h>

typedef float    f32;
typedef double   f64;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int32_t  i32;
typedef bool     b32;

#define CLOCK_SPEED_BASE 2300000000			// 2.3GHz

const u32 FILE_SIZE = 102400000;			// Size in bytes

// Thread function 1
DWORD WINAPI Thread1Func(LPVOID lpParam)
{
    char* pMapView = (char*)lpParam;  // Cast lpParam back to a char pointer
    for (u32 i = 0; i < FILE_SIZE / 2; i++)
    {
        pMapView[i] = 70;  // ASCII char 'F'
    }
    return 0;
}

// Thread function 2
DWORD WINAPI Thread2Func(LPVOID lpParam)
{
    char* pMapView = (char*)lpParam;  // Cast lpParam back to a char pointer
    for (u32 i = FILE_SIZE / 2; i < FILE_SIZE; i++)
    {
        pMapView[i] = 70;  // ASCII char 'F'
    }
    return 0;
}

i32 main(int argc, char* argv[])
{
	HANDLE thread1, thread2;  // Thread handles
    DWORD threadID1, threadID2;

	i32 fildes = open("testFile.txt", O_RDWR | O_CREAT, 00666);

	if (fildes == -1)
	{
		perror("open");
		exit(EXIT_FAILURE);
	}

	u64 startTruncTicks = __builtin_ia32_rdtsc();			// Only for GCC, for MSVC use __rdtsc()
	// Set initial size
	if (ftruncate(fildes, FILE_SIZE) == -1)
	{
		perror("ftruncate");
		_close(fildes);
		exit(EXIT_FAILURE);
	}
	u64 endTruncTicks = __builtin_ia32_rdtsc();

	u64 startMmapTicks = __builtin_ia32_rdtsc();

	// map the initial size of the file. PS: This is only for windows. For Linux, use mmap.
	HANDLE hFile = (HANDLE)_get_osfhandle(fildes);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		printf("Failed to get HANDLE from file descriptor (Error: %lu)\n", GetLastError());
		_close(fildes);
		return 1;
	}

	// Create a file mapping object
	HANDLE hMapFile  =CreateFileMapping(
		hFile,
		NULL,
		PAGE_READWRITE,
		0,
		0,
		NULL
	);
	if (hMapFile == NULL)
	{
		printf("Failed to create file mapping (Error: %lu)\n", GetLastError());
		_close(fildes);
		return 1;
	}

	// Map a view of the file into memory
	char *pMapView = (char *)MapViewOfFile(
		hMapFile,
		FILE_MAP_ALL_ACCESS,
		0,
		0,
		0
	);
	if (pMapView == NULL)
	{
		printf("Failed to map view of file (Error: %lu)\n", GetLastError());
		CloseHandle(hMapFile);
		_close(fildes);
		return 1;
	}
	u64 endMmapTicks = __builtin_ia32_rdtsc();



	// Print to file
	u64 startWrite = __builtin_ia32_rdtsc();
	
	// Create threads, passing pMapView as the parameter
    thread1 = CreateThread(NULL, 0, Thread1Func, (LPVOID)pMapView, 0, &threadID1);
    if (thread1 == NULL)
    {
        printf("Error creating thread 1\n");
        UnmapViewOfFile(pMapView);
        CloseHandle(hMapFile);
        _close(fildes);
        return 1;
    }

    thread2 = CreateThread(NULL, 0, Thread2Func, (LPVOID)pMapView, 0, &threadID2);
    if (thread2 == NULL)
    {
        printf("Error creating thread 2\n");
        CloseHandle(thread1);
        UnmapViewOfFile(pMapView);
        CloseHandle(hMapFile);
        _close(fildes);
        return 1;
    }

     // Wait for threads to finish
    WaitForSingleObject(thread1, INFINITE);
    WaitForSingleObject(thread2, INFINITE);

    // Close thread handles
    CloseHandle(thread1);
    CloseHandle(thread2);
	
	u64 endWrite = __builtin_ia32_rdtsc();


	// Unmap the view and close Handles
	UnmapViewOfFile(pMapView);
	CloseHandle(hMapFile);
	_close(fildes);

	//printf("truncating %u bytes took: %llu ticks\n",FILE_SIZE, endTruncTicks - startTruncTicks);
	printf("truncating %u bytes took: %f ms\n",
		FILE_SIZE, ((endTruncTicks - startTruncTicks)/(float)CLOCK_SPEED_BASE)*1000);

	//printf("mmap %u bytes took: %llu ticks\n",FILE_SIZE, endMmapTicks - startMmapTicks);
	printf("mmap %u bytes took: %f ms\n",
		FILE_SIZE, ((endMmapTicks - startMmapTicks)/(float)CLOCK_SPEED_BASE)*1000);

	//printf("writing %u bytes took: %llu ticks\n",FILE_SIZE, endWrite - startWrite);
	printf("writing %u bytes took: %f ms\n",
		FILE_SIZE, ((endWrite - startWrite)/(float)CLOCK_SPEED_BASE)*1000);

	return 0;
}