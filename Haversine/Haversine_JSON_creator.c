#include <stdio.h>
#include <math.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

typedef float    f32;
typedef double   f64;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int32_t  s32;
typedef bool     b32;

#define BUFF_LEN 100000										// Length of buffer (Fits L1 cache)
struct text_buffer
{
	char text[BUFF_LEN];
	u32  chars_in_buffer;
};

struct pairs
{
	f32 x0;
	f32 y0;
	f32 x1;
	f32 y1;
};

const s32 min_lat_coord  = -180;							// Latitude limits
const s32 max_lat_coord  = 180;								
const s32 min_long_coord = -90;								// Longitude limits
const s32 max_long_coord = 90;
u32 clear_count;											// How many times the buffer has been cleared
b32 isLast;													// If it is the last set of pairs in the loop

b32 pairs_to_JSON(struct pairs* hav_pairs, char* out_pairs_string, u32* len);
f32 random_coord(b32 isLong);
void write_file(struct text_buffer* buffer, struct pairs* haversine_pairs, FILE* file1);


s32 main(int argc, char** argv)
{
	u64 startTicks, endTicks;								// Store the start and end sticks
	u32 bytes;												// Number of bytes written to file
	const u32 point_count = 1000000;						// Number of points to generate
	struct text_buffer buffer;
	struct pairs haversine_pairs;
	buffer.chars_in_buffer = 0;
	
	srand((u32)time(NULL));									// Set the random seed
	
	char starting_format_JSON[30] = "{\n\t\"pairs\":[\n";	// Starting segment of the JSON
	char ending_format_JSON[30] = "\t]\n}";					// Closing segment of the JSON
	
	startTicks = __builtin_ia32_rdtsc();					// Get current Ticks from GCC compiler
	FILE* f1 = fopen("points.JSON","wb");					// Open file
	if(f1 == NULL)
	{
		printf("Error opening file");
		return -1;
	}

	bytes = fwrite(starting_format_JSON, sizeof(char),30, f1);
	if (bytes == 0)
	{
		printf("Error writting to file");
	}

	for(u32 i=0; i<point_count;i++)							// Create the number of datapoints
	{
		if(i == point_count-1) 	isLast = true;				// Check if its the last datapoint						

		write_file(&buffer, &haversine_pairs, f1);

	}

	if(buffer.chars_in_buffer > 0)							// If any text is left in the buffer, write it to file
	{
		fwrite(buffer.text, sizeof(char), buffer.chars_in_buffer, f1);
		buffer.chars_in_buffer = 0;							// Reset the count
	}

	fwrite(ending_format_JSON, sizeof(char),30, f1);
	if (bytes == 0)
	{
		printf("Error writting to file");
	}

	fclose(f1);
	endTicks = __builtin_ia32_rdtsc();
	printf("Buffer cleared %u times\n", clear_count);
	printf("Algo took: %llu ticks %f ms", endTicks - startTicks, (endTicks - startTicks)/(f32)2400000);
	return 0;
}

b32 pairs_to_JSON(struct pairs* hav_pairs, char* out_pairs_string, u32* len)
{
	*len = snprintf(out_pairs_string, BUFF_LEN, "\t\t{\"x0\":%f, \"y0\":%f, \"x1\":%f, \"y1\":%f}%s\n",
                hav_pairs->x0, hav_pairs->y0, hav_pairs->x1, hav_pairs->y1,
                isLast ? "" : ",");

	return *len > 0;
}

f32 random_coord(b32 isLong)
{
	if(isLong)
	{
		return (max_long_coord - min_long_coord) * ((f32)rand() / RAND_MAX) + min_long_coord;
	}
	else
	{
		return (max_lat_coord - min_lat_coord) * ((f32)rand() / RAND_MAX) + min_lat_coord;
	}
}

void write_file(struct text_buffer* buffer, struct pairs* haversine_pairs, FILE* file1)
{
	char text[100];											// char array to store the haversine coordinate pairs after formatting
	u32  charlen;											// Length of the char array

	haversine_pairs->x0 = random_coord(false);				// Generate the coordinates
	haversine_pairs->y0 = random_coord(true);
	haversine_pairs->x1 = random_coord(false);
	haversine_pairs->y1 = random_coord(true);

	b32 success = pairs_to_JSON(haversine_pairs, text, &charlen);
	if (!success)
	{
		printf("Error while creating string with haversine_pairs");
		return;
	}

	if((BUFF_LEN - buffer->chars_in_buffer) > charlen)		// If there is space in buffer, add the text
	{
		for(u32 j=0; j<charlen;j++)							// Append the char[] to the end of the buffer
		{
			buffer->text[buffer->chars_in_buffer + j] = text[j];

		}

		buffer->chars_in_buffer += charlen;					// Update the number of chars in the buff
	}
	else													// If not, clear it first by writting to file
	{
		u32 bytes = fwrite(buffer->text, sizeof(char), buffer->chars_in_buffer, file1);
		if (bytes == 0)
		{
			printf("Error writting to file");
		}
		else
		{
			buffer->chars_in_buffer = 0;					// Reset the count
			clear_count +=1;								// Increment the clear count

			for(u32 j=0; j<charlen;j++)						// Append the char[] to the end of the buffer
			{
				buffer->text[buffer->chars_in_buffer + j] = text[j];
	
			}

			buffer->chars_in_buffer += charlen;				// Update the number of chars in the buff
		}
	}
}