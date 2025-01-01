#include <stdio.h>
#include <windows.h>
#include <time.h>
#include <stdlib.h>

typedef long long u64;

#define ARRAY_SIZE 100000
#define CLOCK_SPEED 2400000000

void sortArray(int array[], int size)
{
	for(int i=0;i<size-1; i++)
	{
		for(int j=0;j<size-i-1; j++)
		{
			int temp=0;
			if(array[j]>array[j+1])
			{
				temp = array[j];
				array[j] = array[j+1];
				array[j+1] = temp;
			}
		}	
	}
}


int main(int argc, char* argv[])
{
	// int budget = 5000;
	// int arr[] = {500,300,100,400,900,1000,200,600,2000,700,850,3000};

	// int size = sizeof(arr)/sizeof(arr[0]);
	

	srand(time(NULL));   // Initialization, should only be called once.
	int* array = (int* )malloc(ARRAY_SIZE * sizeof(int));

	for(int i=0; i<ARRAY_SIZE; i++)
	{
		array[i] = rand();
	}
	
	u64 startTick = __rdtsc();
	sortArray(array, ARRAY_SIZE);	
	u64 stopTick = __rdtsc();
	
	float time = (stopTick-startTick) / (float)CLOCK_SPEED;
	printf("Sorting %d numbers with Bubble Sort took:\n%f seconds\n", ARRAY_SIZE, time) ;
	free(array);

	// int housesBought = 0;
	// for(int i=0; i<size; i++)
	// {
	// 	if(arr[i]<budget)
	// 	{
	// 		budget -= arr[i];
	// 		housesBought++;
	// 	}
	// 	else
	// 	{
	// 		break;
	// 	}
	// }

	//printf("Houses that can be bought: %d\n", housesBought);
}