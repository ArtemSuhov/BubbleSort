#include "pch.h"
#include <iostream>
#pragma once

template< typename T >
T* bubbleSorted(T* array, int size) 
{
	T* sortedArray = new T[size];
	memcpy(sortedArray, array, sizeof(T) * size);

	for (int i = 0; i < size - 1; i++) {
		for (int j = size - 1; j > i; j--) {
			if (sortedArray[j] < sortedArray[j - 1])
			{
				T temporary = sortedArray[j];
				sortedArray[j] = sortedArray[j - 1];
				sortedArray[j - 1] = temporary;
			}
		}
	}

	return sortedArray;
}

int main()
{
	double array[5] = { 1,3,1,6,4 };
	double* sortedArray = bubbleSorted(array, 5);
	for (int i = 0; i < 5; i++) {
		printf("%f ", sortedArray[i]);
	}
}
