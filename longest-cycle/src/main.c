/*
Dean Babić
0036470256
*/
#include <stdio.h>  
#include <stdlib.h>
#include <string.h> // needed for reading input file

#include <limits.h> // needed to check bounding conditions
#include <errno.h>  // needed for error codes
#if defined(_WIN32) || defined(_WIN64)
/* We are on Windows */
# define strtok_r strtok_s
# include <io.h> // needed to check file availability
#endif
#if defined(_UNIX)
#include <unistd.h> // needed to check file availability
#endif
void PrepareNeighborHoodMatrix(int argc, char* argv[]);
char* ReadFile(char *argv);
void ReadNeighborhoodMatrix(char *readFile);
void PrintMatrix();
void PrintCycle(int* cycle, int length);
int FindLongestCycle(int** cycle);
int BFS(int currentVertice, int beginVertice, int length, int maxLength, int *visited, int** list);
void ErrorHandler(int errorCode);
void Destructor();

int noOfVertices;
int **neighborhood;

const int base = 10;

// Main method, it calls other methods
int main(int argc, char* argv[])
{
	int length;
	// Read the neighborhood matrix
	PrepareNeighborHoodMatrix(argc, argv);
	int* cycle = (int*)malloc((noOfVertices + 1) * sizeof(int));
	// Find the longest cycle
	length = FindLongestCycle(&cycle);
	printf("%d\n", length);
	// Clear the heap allocated memory
	PrintCycle(cycle, length + 1);
	Destructor();
	return 0;
}

// Ensures that everything is correct before the longest cycle can be found
void PrepareNeighborHoodMatrix(int argc, char* argv[])
{
	char *readFile;
	// Check if the user has given input parameters
	if (argc < 2)
		ErrorHandler(-1);
	// Check if the file is accessible or read protected
	#if defined(_WIN32) || defined(_WIN64)
		if (_access(argv[1], 0))
			ErrorHandler(-2);
	#endif
	#if defined(_UNIX)
		if (access(argv[1], F_OK | R_OK))
			ErrorHandler(-2);
	#endif
	// Check if there are any errors whilst reading the file
	readFile = ReadFile(argv[1]);
	if (readFile == NULL)
		ErrorHandler(-3);
	// Now fill the neghborhood matrix
	ReadNeighborhoodMatrix(readFile);
}

// Reads file and stores it into a char array
char*  ReadFile(char *filename)
{
	char *buffer = NULL;
	int string_size, read_size;
	FILE *handler = fopen(filename, "rb");

	if (handler)
	{
		fseek(handler, 0, SEEK_END);
		string_size = ftell(handler);
		rewind(handler);

		buffer = (char*)malloc(sizeof(char) * (string_size + 1));
		read_size = fread(buffer, sizeof(char), string_size, handler);
		buffer[string_size] = '\0';

		if (string_size != read_size)
		{
			free(buffer);
			buffer = NULL;
		}
		fclose(handler);
	}
	return buffer;
}

// Fills neighborhood matrix with values contained in char array
void ReadNeighborhoodMatrix(char *readFile)
{
	char *token;
	char *str = readFile;
	token = strtok_r(str, "\n\r", &str);
	char *endptr;
	long result = strtol(token, &endptr, base);
	if (endptr == token || ((result >= INT_MAX || result <= INT_MIN) && errno == ERANGE))
	{
		free(readFile);
		ErrorHandler(-4);
	}

	noOfVertices = (int)result;
	neighborhood = (int**)malloc(noOfVertices * sizeof(int*));
	for (int i = 0; i < noOfVertices; i++)
		neighborhood[i] = (int*)malloc(noOfVertices * sizeof(int));

	for (int i = 0; i < noOfVertices; i++)
	{
		token = strtok_r(str, "\n\r", &str);
		if (token == NULL || strlen(token) == 0)
		{
			free(readFile);
			ErrorHandler(-6);
		}

		char *strcopy = (char*)calloc(strlen(token) + 1, sizeof(char));
		strcpy(strcopy, token);
		char *elem = strcopy;
		for (int j = 0; j < noOfVertices; j++)
		{
			token = strtok_r(elem, " ", &elem);
			if (token == NULL || strlen(token) == 0)
			{
				free(readFile);
				free(strcopy);
				ErrorHandler(-6);
			}

			result = strtol(token, &endptr, base);
			if (endptr == token || ((result >= INT_MAX || result <= INT_MIN) && errno == ERANGE))
			{
				free(readFile);
				free(strcopy);
				ErrorHandler(-5);
			}

			neighborhood[i][j] = (int)result;
		}
		free(strcopy);
	}
}

// Test to see if the matrix loaded correctly
void PrintMatrix()
{
	for (int i = 0; i < noOfVertices; i++)
	{
		for (int j = 0; j < noOfVertices; j++)
		{
			printf("%d ", neighborhood[i][j]);
		}
		printf("\n");
	}
}

void PrintCycle(int* cycle, int length)
{
	for(int i = 0; i < length; i++)
	{
		printf("%d ", cycle[i]);
	}
	printf("\n");
}

/*
Looks inside of neighborhood matrix to find cycles
Returns maximum cycle length
*/
int FindLongestCycle(int** cycle)
{
	int length = 0;
	int *visited = (int*)calloc(noOfVertices, sizeof(int));
	int *tempCycle = (int*)malloc((noOfVertices + 1) * sizeof(int));
	
	// Iterate over all nodes and set them as start nodes, some nodes may be leaf nodes
	// or they may be inside of a smaller cycle
	for (int currentVertice = 0; currentVertice < noOfVertices; currentVertice++)
	{
		// Call recursive Breadth First Search method to try and find longest cycle.
		// If it is longer than current maximum, update maximum length
		tempCycle[0] = currentVertice;
		int currentLength = BFS(currentVertice, currentVertice, 0, length, visited, &tempCycle);
		if (currentLength > length)
		{
			length = currentLength;
			for(int i = 0; i < length + 1; i++)
			{
				(*cycle)[i] = tempCycle[i];
			}
		}
	}

	free(visited);
	return length;
}

/*
Recursive method for traversing the tree
In this case graph in some way we can perceive tree as unraveled graph where leaf nodes
represent either a root node or some node that closes a cycle or simply leaf node of a graph
Returns new maximum length
*/
int BFS(int currentVertice, int beginVertice, int length, int maxLength, int *visited, int** list)
{
	int newLength = maxLength;
	visited[currentVertice] = 1;

	for (int nextVertice = 0; nextVertice < noOfVertices; nextVertice++)
	{
		if (neighborhood[currentVertice][nextVertice] == 1)
		{
			// If next node is root node then update maximum length if new length is greater
			if (nextVertice == beginVertice && length > 1 && length + 1 > maxLength)
			{
				newLength = length + 1;
				(*list)[length] = nextVertice;
			}
			// If next node hasn't yet been visited recursivelly go into that node
			else if (!visited[nextVertice])
			{
				(*list)[length + 1] = nextVertice;
				newLength = BFS(nextVertice, beginVertice, length + 1, newLength, visited, list);
			}
		}
	}
	// After there are no more legitimate moves, return new maximum length and remove
	// your node from visited nodes
	visited[currentVertice] = 0;
	return newLength;
}

// Describes the type of error to the user and handles the termination of program
void ErrorHandler(int errorCode)
{
	char *errorText;
	switch (errorCode)
	{
	case -1:
		errorText = "Not enough input parametres";
		break;
	case -2:
		errorText = "The file does not exit or is not readable";
		break;
	case -3:
		errorText = "Error whilst reading the file";
		break;
	case -4:
		errorText = "Wrong vertice number, check input file";
		break;
	case -5:
		errorText = "Wrong weight number inside of the neighborhood matrix, check input file";
		break;
	case -6:
		errorText = "Wrong width or height of the neighborhood matrix, check input file";
		break;
	default:
		errorText = "Unknown error occured";
		break;
	}
	printf("%s\n", errorText);
	Destructor();
	exit(errorCode);
}

// Handles memory deallocation when the program is exiting
void Destructor()
{
	for (int i = 0; i < noOfVertices; i++) {
		free(neighborhood[i]);
	}
	free(neighborhood);
}