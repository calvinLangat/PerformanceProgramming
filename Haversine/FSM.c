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

enum ParseState {
	ERROR_ENCOUNTERED = -1,
	SEARCHING_FOR_ENTITY = 0,
	FOUND_ENTITY,
	SEARCHING_FOR_ELEMENT,
	FOUND_ELEMENT,
	READING_VALUE,
	END_OF_ARRAY,
	END_OF_FILE,
};

enum EntityType {
	ARRAY
};

enum ParseResult {
	SUCCESS,
	FAILED,
	NOT_FOUND
};

u32 entity_count = 0;
u32 elements_found_count = 0;
u32 read_values_count = 0;
enum ParseState state;

enum ParseResult getElement(
	char* json_file,
	u32 file_len,
	enum EntityType type,
	const char* entity_name,
	const char* element_name,
	f32* out_element_array,
	u32 array_size,
	u32* out_elementCount)
{
	state = SEARCHING_FOR_ENTITY;
	bool encountered_starting_brace = false;
	bool encountered_quotes = false;
	bool encountered_braces = false;
	bool encountered_sq_brackets = false;
	bool inArray = false;

	char name[256] = {0};
	char value[256] = {0};
	char suffix[1] = {0};

	if (type == ARRAY)
	{
		suffix[0] = '[';
	}

	// Hardcode for now
	suffix[0] = '[';


	// TODO: Check array_count after every insert
	for (u32 i = 0; i < file_len; i++)
	{
		if (state == SEARCHING_FOR_ENTITY)
		{
			// When we encounter a quote for the first time
			if (json_file[i] == '"')
			{
				encountered_quotes = true;
				u32 j = 0;
				while (encountered_quotes && i < file_len)
				{
					// Move to the next character
					i++;
					if (json_file[i] == '"')			// If we encounter another quote, means that the word has ended
					{
						encountered_quotes = false;
						name[j] = '\0';					// Zero terminate it
						break;							// Exit the loop
					}

					name[j] = json_file[i];
					j++;
				}

			}

			
			// Check if the name found matches what we want
			if (strcmp(name, entity_name) == 0)
			{
				// If match, check the next two characters for JSON format ':[' for start of array
				if (json_file[i + 1] == ':' && json_file[i + 2] == suffix[0])
				{
					state = FOUND_ENTITY;
					encountered_sq_brackets = true;
					entity_count++;
					i += 3;								// Skip ahead 3 positions since we have confirmed what these characters are
				}
				else
				{
					// Bad formatting
					return NOT_FOUND;
				}
			}
		}


		if (state == FOUND_ENTITY)
		{
			state = SEARCHING_FOR_ELEMENT;

		}

		while (state == SEARCHING_FOR_ELEMENT && i < file_len)
		{
			if (json_file[i] == '{')					// Check start of array
			{
				inArray = true;
				i++;
			}

			// Check for our key
			if (json_file[i] == '"' &&
				json_file[i + 1] == element_name[0] &&
				json_file[i + 2] == element_name[1] &&
				json_file[i + 3] == '"')
			{
				state = FOUND_ELEMENT;
				i+=4;
				break;
			}

			if (json_file[i] == '}') {					// Check if we are at the end of the array
				inArray = false;
				i++;
				break;
			}
			if (i >= file_len)							// Check if end of file
			{
				state = END_OF_FILE;
				break;
			}

			i++;

		}

		while (state == FOUND_ELEMENT && i < file_len)
		{
			if (json_file[i] == ':')
			{
				state = READING_VALUE;
				i++;
				break;
			}
			else
			{
				i++;
			}
		}

		u32 k = 0;
		while (state == READING_VALUE && i < file_len)
		{
			// Read value until we encounter a n ',' or '}'
			if (json_file[i] != ',' && json_file[i] != '}')
			{
				value[k] = json_file[i];
				k++;
				i++;
			}
			else
			{
				// Make sure we don't exceed array limits
				if ((*out_elementCount) < array_size)
				{
					value[k] = '\0';
					out_element_array[*out_elementCount] = atof(value);
					(*out_elementCount)++;
				}
				else
				{
					printf_s("Values too many to fit allocated array\n");
					state = ERROR_ENCOUNTERED;
					return FAILED;
				}
				// Update state back to SEARCHING
				state = SEARCHING_FOR_ELEMENT;
				i++;
				break;
			}
		}
	}

	state = END_OF_FILE;

	return SUCCESS;

}

bool createMemoryMap(const s32* file_descriptor, char** mapview, HANDLE* hMapFile)
{
	// Get windows file handle from file descriptor
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

	u64 start_tick = __rdtsc();

	if (stat(file, &sb))
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

	// Create memory mapped file
	if (!createMemoryMap(&file_Desc, &pMapView, &hMapFile))
	{
		printf_s("Failed to create Memory map. (Error: %lu)\n", GetLastError());
		return -1;
	}

	// Allocate arrays of ! million datapoints
	const u32 array_size = 1000000;
	f32* array_x0 = (f32*)malloc(array_size * sizeof(f32));
	u32 elementCount_x0 = 0;
	f32* array_y0 = (f32*)malloc(array_size * sizeof(f32));
	u32 elementCount_y0 = 0;
	f32* array_x1 = (f32*)malloc(array_size * sizeof(f32));
	u32 elementCount_x1 = 0;
	f32* array_y1 = (f32*)malloc(array_size * sizeof(f32));
	u32 elementCount_y1 = 0;

	if (array_x0 == NULL || array_x1 == NULL || array_y0 == NULL || array_y1 == NULL)
	{
		printf_s("Failure to allocate memory\n");
		return -1;
	}

	enum ParseResult result;
	result = getElement(pMapView, fileSize, ARRAY, "pairs", "x0", array_x0, array_size, &elementCount_x0);

	if (result != SUCCESS)
	{
		printf_s("Error\n");
	}

	result = getElement(pMapView, fileSize, ARRAY, "pairs", "x1", array_x1, array_size, &elementCount_x1);

	if (result != SUCCESS)
	{
		printf_s("Error\n");
	}

	result = getElement(pMapView, fileSize, ARRAY, "pairs", "y0", array_y0, array_size, &elementCount_y0);

	if (result != SUCCESS)
	{
		printf_s("Error\n");
	}

	result = getElement(pMapView, fileSize, ARRAY, "pairs", "y1", array_y1, array_size, &elementCount_y1);

	if (result != SUCCESS)
	{
		printf_s("Error\n");
	}

	u64 stop_tick = __rdtsc();

	printf_s("%f\n", array_x0[99]);
	printf_s("%f\n", array_x1[99]);
	printf_s("%f\n", array_y0[99]);
	printf_s("%f\n", array_y1[99]);

	printf_s("Parse took: %llu ticks\n", stop_tick - start_tick);
	printf_s("Parse took: %f ms\n", (stop_tick - start_tick) / (double)2300000);


	printf_s("Press ENTER key to Continue\n");
	getchar();


	// Clean up
	// Unmap the view and close Handles
	free(array_x0);
	UnmapViewOfFile(pMapView);
	CloseHandle(hMapFile);
	close(file_Desc);
	return 0;
}