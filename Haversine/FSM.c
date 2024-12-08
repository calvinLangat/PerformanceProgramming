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

enum ParseState{
	SEARCHING_FOR_ENTITY,
	FOUND_ENTITY,
	SEARCHING_FOR_ELEMENT,
	FOUND_ELEMENT,
	READING_VALUE,
	END_OF_ARRAY,
	END_OF_FILE
};

enum EntityType{
	ARRAY
};

enum ParseResult{
	SUCCESS,
	FAILED,
	NOT_FOUND
};

u32 entity_count = 0;
u32 elements_found_count = 0;
u32 read_values_count = 0;
enum ParseState state;

enum ParseResult getElement(char* json_file, u32 file_len, enum EntityType type, const char* entity_name, const char* element_name, f32* element_array, u32 array_count)
{
	state = SEARCHING_FOR_ENTITY;
	bool encountered_starting_brace = false;
	bool encountered_quotes = false;
	bool encountered_braces = false;
	bool encountered_sq_brackets = false;

	char name[256];
	char suffix[1];

	if (type == ARRAY)
	{
		suffix[0] = '[';
	}


	// TODO: Check array_count after every insert
	for(u32 i=0; i<file_len; i++)
	{	
		if(state == SEARCHING_FOR_ENTITY)
		{
			// When we encounter a quote for the first time
			if(json_file[i] == '"')
			{
				encountered_quotes = true;
				u32 j = 0;
				while(encountered_quotes)
				{
					// Move to the next character
					i++;
					if(json_file[i] == '"')			// If we encounter another quote, means that the word has ended
					{
						encountered_quotes = false;
						name[j+1] = '\0';			// Zero terminate it
						break;						// Exit the loop
					}

					name[j] = json_file[i];
					j++;
				}

			}
			
			// Check if the name found matches what we want
			if(strcmp(name, entity_name) == 0)
			{
				// If match, check the next two characters for JSON format ':[' for start of array
				if (json_file[i+1] == ':' && json_file[i+2] == suffix[0])
				{
					state = FOUND_ENTITY;
					encountered_sq_brackets = true;
					entity_count++;
					i+=3;							// Skip ahead 3 positions since we have confirmed what these characters are
				}
			}
		}
		

		if (state == FOUND_ENTITY)
		{
			state = SEARCHING_FOR_ELEMENT;
		}

		if(state == SEARCHING_FOR_ELEMENT)
		{

		}
	}

	state = END_OF_FILE;

	return SUCCESS;

}

bool createMemoryMap(const s32* file_descriptor, char** mapview, HANDLE* hMapFile)
{
	HANDLE hFile = (HANDLE)_get_osfhandle(*file_descriptor);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		printf("Failed to get HANDLE from file descriptor (Error: %lu)\n", GetLastError());
		close(*file_descriptor);
		return false;
	}

	// Create space in memory equal to the size of the file (rounded up to the nearest 4K bytes)
	hMapFile = CreateFileMappingA(
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
		close(*file_descriptor);
		return false;
	}

	// Map View of file
	*mapview = (char*)MapViewOfFile(
		hMapFile,
		FILE_MAP_ALL_ACCESS,
		0,
		0,
		0
	);

	if (*mapview == NULL)
	{
		printf("Failed to map view of file (Error: %lu)\n", GetLastError());
		CloseHandle(hMapFile);
		close(*file_descriptor);
		return false;
	}

	return true;
}

int main(int argc, char* argv[])
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

	char* pMapView = NULL;
	HANDLE hMapFile = INVALID_HANDLE_VALUE;
	
	if(!createMemoryMap(&file_Desc, &pMapView, &hMapFile))
	{
		printf_s("Failed to create Memory map. (Error: %lu)\n", GetLastError());
		return -1;
	}
	
	const u32 array_size = 4096;
	f32* array_x0 = (f32*)malloc(array_size * sizeof(f32));

	enum ParseResult result = getElement(pMapView, fileSize, ARRAY, "pairs", "x0", array_x0, array_size);

	if(result == SUCCESS)
	{
		printf("Count of Entities found: %d\nParse state: %d", entity_count, state);
	}

	// Clean up
	// Unmap the view and close Handles
	free(array_x0);
	UnmapViewOfFile(pMapView);
	CloseHandle(hMapFile);
	close(file_Desc);
	return 0;
}