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

typedef struct
{
    int numVertices;
    int** neighborhood;
    int* colors;
} Graph;

void PrepareNeighborHoodMatrix(int argc, char* argv[]);
char* ReadFile(char *argv);
void ReadNeighborhoodMatrix(char *readFile);
Graph CopyGraph(Graph input);
int DegreeGreater();
void PrintMatrix();
int Is3Colorable();
int CheckVertexDegree();
int LayerColor(int vertice);
int* GetNeighbors(int vertice);
void ErrorHandler(int errorCode);
void Destructor();

Graph G;

const int base = 10;

// Main method, it calls other methods
int main(int argc, char* argv[])
{
	int colorable;
	// Read the neighborhood matrix
	PrepareNeighborHoodMatrix(argc, argv);
	// Find the longest cycle
	colorable = Is3Colorable();
	printf("%d\n", colorable);
	// Clear the heap allocated memory
	Destructor(G);
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
	char *str = readFile;
	char buffer[100];
	int result;
	int size = sscanf(str, "%d\n", &result);
	if (size == 0)
	{
		free(readFile);
		ErrorHandler(-4);
	}
	G.numVertices = result;
	str = strchr(str, '\n') + 1;
	str = strchr(str, '\n') + 1;
	G.neighborhood = (int**)malloc(G.numVertices * sizeof(int*));
	for (int i = 0; i < G.numVertices; i++)
		G.neighborhood[i] = (int*)malloc(G.numVertices * sizeof(int));
    G.colors = (int*)calloc(G.numVertices + 1, sizeof(int));

	for (int i = 0; i < G.numVertices; i++)
	{
		size = sscanf(str, "%[^\0]", buffer);
		//token = strtok_r(str, "\n\r", &str);
		if (size == 0)
		{
			free(readFile);
			ErrorHandler(-6);
		}
		char* ptr = buffer;
		for (int j = 0; j < G.numVertices; j++)
		{
			size = sscanf(ptr, "%d", &result);
			if (size == 0)
			{
				free(readFile);
				ErrorHandler(-6);
			}
			G.neighborhood[i][j] = result;
			ptr = strchr(ptr, ' ') + 1;
		}
		str = strchr(str, '\n') + 1;
	}
}

// Test to see if the matrix loaded correctly
void PrintMatrix()
{
	for (int i = 0; i < G.numVertices; i++)
	{
		for (int j = 0; j < G.numVertices; j++)
		{
			printf("%d ", G.neighborhood[i][j]);
		}
		printf("\n");
	}
}

/*
Looks inside of neighborhood matrix to calculate Chromatic number
Returns 0 if not 3-colorable 1 otherwise
*/
int Is3Colorable()
{
    // First we try the smart way to determine if it is not 3-colorable by checking the vertex degrees
    // If any vertex has a degree greater than 3, well then it is impossible to use only three collors
    // for those 4 or more neighboring vertices
	int colorable = CheckVertexDegree();
    if(!colorable)
        return colorable;
    
    // Now we brute force our colors to see if the graph is 3-colorable
    colorable = LayerColor(0);

	return colorable;
}

/*
This method checks to see if there is any vertex degree that is greater than 2.
Since if we have degree 3 or greater we must have Chromatic number equal to 4 or greater.
If G is simple non-complete connected graph, and if the gratest degree of some vertex is equal to ∆ (∆ ≥ 3), then G is ∆-colorable.
Returns 0 if not 3-colorable, 1 if it may be colorable
*/
int CheckVertexDegree()
{
    int colorable = 1;
    // We iterate through all the vertices
    for (int currentVertice = 0; currentVertice < G.numVertices; currentVertice++)
	{
        int degree = 0;
        for(int i = 0; i < G.numVertices; i++)
        {
            // And calculate how many neighbors they have
            if(G.neighborhood[currentVertice][i] == 1)
                degree++;
        }
        if(degree > 3)
        {
            colorable = 0;
            break;
        }
    }
    return colorable;
}

/*
Recursive method for coloring the vertices
Returns 0 if not 3-colorable, 1 if it may be colorable
*/
int LayerColor(int vertice)
{
    int colorable = 0;
    int success;

    // We need to have all the neighbors of the current vertice
    int* listNeighbors = GetNeighbors(vertice);
    int* availableColors = calloc(4, sizeof(int));
    availableColors[1] = availableColors[2] = availableColors[3] = 1; 
    
    // So that we could see what colors remain for this particular vertice
    for (int i = 0; i < G.numVertices; i++)
    {
        if(listNeighbors[i] == 1 && G.colors[i] != 0)
            availableColors[G.colors[i]] = 0;
    }

    // Now we iterate through the colors 1 till 3, 0 represents no color
    for(int i = 1; i < 4; i++)
    {
        if(availableColors[i] == 0)
            continue;
        success = 1;
        G.colors[vertice] = i;

        // After we have assigned the color for this vertice we recursively need to color other neighboring vertices
        for (int i = 0; i < G.numVertices; i++)
        {
            if(listNeighbors[i] == 0)
                continue;
            if(G.colors[i] == 0)
                success = LayerColor(i);
            if(success == 0)
                break;
        }
        if(success == 1)
            break;
    }
    
    // If it is successful then we can say that it is colorable
    if(G.colors[vertice] != 0 && success == 1)
        colorable = 1;
    // Otherwise we need to remove that color and try with different one
    if(success == 0)
        G.colors[vertice] = 0;

    free(listNeighbors);
    free(availableColors);
   
	return colorable;
}

// Returns the neighboring vertices of the current one
int* GetNeighbors(int vertice)
{
    int* neighbors = (int*) calloc(G.numVertices, sizeof(int));
    for(int i = 0; i < G.numVertices; i++)
    {
        if(G.neighborhood[vertice][i] == 1)
            neighbors[i] = 1;
    }
    return neighbors;
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
	for (int i = 0; i < G.numVertices; i++) {
		free(G.neighborhood[i]);
	}
	free(G.neighborhood);
}