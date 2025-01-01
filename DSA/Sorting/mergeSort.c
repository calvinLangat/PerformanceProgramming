#include <stdio.h>
#include <windows.h>
#include <time.h>
#include <stdlib.h>

void merge_sort(int array[], int length);
void merge_sort_recursion(int array[], int left_index, int right_index);
void merge_sorted_array(int array[], int left_index, int middle_index, int right_index);

#define ARRAY_SIZE 100000
#define CLOCK_SPEED 2400000000

int main(int argc, char* argv[])
{
	//int budget = 5000;
	//int arr[] = {500,300,100,400,900,1000,200,600,2000,700,850,3000};
	//int size = sizeof(arr)/sizeof(arr[0]);
	
	srand(time(NULL));   // Initialization, should only be called once.
	int* array = (int* )malloc(ARRAY_SIZE * sizeof(int));

	for(int i=0; i< ARRAY_SIZE; i++)
	{
		array[i] = rand();
	}
	
	long long startTick = __rdtsc();
	merge_sort(array, ARRAY_SIZE);	
	long long stopTick = __rdtsc();

	float time = (stopTick-startTick) / (float)CLOCK_SPEED;
	printf("Sorting %d numbers with Merge Sort took:\n%f seconds\n", ARRAY_SIZE, time) ;


	for(int i=0; i<10; i++)
	{
		printf("%d\n", array[i]);
	}
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

void merge_sort(int array[], int length)
{
	merge_sort_recursion(array, 0, length-1);
}

void merge_sort_recursion(int array[], int left_index, int right_index)
{
	if(left_index < right_index)
	{
		int middle_index = left_index + (right_index - left_index) / 2;
		
		merge_sort_recursion(array, left_index, middle_index);
		merge_sort_recursion(array, middle_index+1, right_index);
		merge_sorted_array(array, left_index, middle_index, right_index);
	}
}

void merge_sorted_array(int array[], int left_index, int middle_index, int right_index)
{
	int left_length = middle_index - left_index + 1;
	int right_length = right_index - middle_index;

	int temp_left[left_length];
	int temp_right[right_length];

	int i=0;
	int j=0;
	int k=0;

	for(int i=0; i<left_length; i++)
	{
		temp_left[i] = array[left_index + i];
	}

	for(int i=0; i<right_length; i++)
	{
		temp_right[i] = array[middle_index + 1 + i];
	}

	for(i=0, j=0, k=left_index; k<=right_index; k++)
	{
		if(i < left_length && (j>=right_length || temp_left[i] <= temp_right[j]))
		{
			array[k] = temp_left[i];
			i++;
		}
		else
		{
			array[k] = temp_right[j];
			j++;
		}
	}

}