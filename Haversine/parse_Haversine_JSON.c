#include <stdio.h>
#include <math.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <windows.h>

typedef float    f32;
typedef double   f64;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int32_t  s32;
typedef bool     b32;

int main()
{

	// Get filesize from file information
	struct stat sb;
	const char* file = "D:/MEGA/Documents/Code/C/Haversine/points.JSON";

	if(stat(file, &sb))
	{
		printf_s("Error getting file info.\n");
		return -1;
	}

	const u32 fileSize = sb.st_size;
	printf_s("File size: %d bytes\n", fileSize);

	// Open file and get file handle
	s32 file_Desc = open(file, O_RDWR);
	if (file_Desc == -1)
	{
		perror("open");
		exit(EXIT_FAILURE);
	}

	HANDLE hFile = (HANDLE)_get_osfhandle(file_Desc);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		printf("Failed to get HANDLE from file descriptor (Error: %lu)\n", GetLastError());
		close(file_Desc);
		return 1;
	}

	// Create space in memory equal to the size of the file (rounded up to the nearest 4K bytes)
	HANDLE hMapFile = CreateFileMappingA(
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
		close(file_Desc);
		return 1;
	}

	// Map View of file
	char* pMapView = (char*)MapViewOfFile(
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
		close(file_Desc);
		return 1;
	}
	
	// Test check how many "x0" can I find
	char token_x0[2] = "x0";
	char token_x1[2] = "x1";
	char token_y0[2] = "y0";
	char token_y1[2] = "y1";
	char colon[1] = ":" ;
	char tokenEnd[1] = ",";
	u32 tokenCount_x0 = 0;
	u32 colonCount = 0;
	u32 commaCount = 0;
	u32 colonLoc[UINT16_MAX];
	u32 commaLoc[UINT16_MAX];

	for(u32 i=0; i<fileSize; i++)
	{
		printf_s("%c", pMapView[i]);

		if(pMapView[i] == token_x0[0] && pMapView[i+1] == token_x0[1])
			tokenCount_x0++;

		if(pMapView[i] == colon[0])
		{
			if(!(pMapView[i+1] == '['))
			{
				colonLoc[colonCount] = i;
				colonCount++;
			}	
		}

		if(pMapView[i] == tokenEnd[0])
		{
			if(pMapView[i-1] == '}')
			{
				commaLoc[commaCount] = i-1;
			}
			else
			{
				commaLoc[commaCount] = i;
			}
			commaCount++;
		}
	}

	printf_s("\nNumber of x0 points: %d\n", tokenCount_x0);
	printf_s("Number of colons: %d\n", colonCount);
	printf_s("Number of commas: %d\n", commaCount);
	
	
	f32* array_x0 = (f32*)malloc(tokenCount_x0 * sizeof(f32));
	
	if(array_x0 == NULL)
	{
		printf_s("Failed to allocate array memory");
		return 1;
	}

	u32 l=0;
	u32 arrLoc   = 0;
	for (u32 k=0; k<tokenCount_x0; k++)
	{
		
		char x0[13];
		u32 charLoc  = 0;
		u32 startLoc = colonLoc[l] + 1;
		u32 endLoc   = commaLoc[l];
		l+=4;

		for(u32 l=startLoc; l<endLoc; l++)
		{
			x0[charLoc] = pMapView[l];
			charLoc++;
		}

		// Null-terminate the string
    	x0[charLoc] = '\0';
		array_x0[arrLoc] = atof(x0);
		arrLoc++;
	}
	

	
	for(u32 i=0; i<tokenCount_x0; i++)
	{
		printf_s("%f\n", array_x0[i]);
	}

	// Unmap the view and close Handles
	free(array_x0);
	UnmapViewOfFile(pMapView);
	CloseHandle(hMapFile);
	close(file_Desc);
	return 0;
}