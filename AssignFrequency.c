#include<stdio.h>
#include<stdint.h>
#include<stdlib.h>
#include<string.h>

#define EMPTY -1
#define MAX 50
#define INFINITY 100000
#define MAXFREQ 100

#define SWAP(a, b) do { a ^= b; b ^= a; a ^= b; } while ( 0 )

int s0, s1, s2, DEBUG;
int C[MAX][MAX] = {{EMPTY}, {EMPTY}};

void assignFrequency (int[2][MAX], int[MAX][MAX], FILE*);
void sort (int[2][MAX], int[2][MAX], int[MAX][MAX]);
void calculateDistance(int[MAX][MAX], int[MAX][MAX]);
void calculateC(int[MAX][MAX]);
void displayXW(int[2][MAX]);
void displayGraph(int[MAX][MAX]);
void findAssignment (int[2][MAX], int[MAX][MAX], int, int, int[2][MAX][MAXFREQ], int[MAX]);

int calculateAdjacentDemandSum (int, int[MAX][MAX], int[MAX][MAX]);
int findIncrement(int[2][MAX], int[2][MAX], int);
int belongsTo(int[MAX], int);
int last(int[2][MAX][MAXFREQ], int);
int maxfr(int[2][MAX]);
int nonzero(int[2][MAX]);

int min(int a, int b) {
	if(a < b)
		return a;
	else
		return b;
}

int main (int argc, char *argv[]) {
		
	if(argc != 4 && argc != 5)
	{
		printf("Incorrect usage! Usage: AssignFrequency [Option: -e (echo on)] [ProblemFile.txt] [GraphFile.txt] [OutputFile.txt]\nExiting...");
		exit(-1);
	}
	
	int XW[2][MAX] = {{EMPTY}, {EMPTY}};			//2D array representing the nodes and their demands
		
	if(argc == 4)
	{
		DEBUG = 0;
	}
	else
	{
		DEBUG = 1;
		argv[1] = argv[2];
		argv[2] = argv[3];
		argv[3] = argv[4];
	}
	
	FILE *f1 = fopen(argv[1], "r");
	if(!f1)
	{	
		printf("\nCould not read from the problem file!");
		exit(-1);
	}
	
	/*
	Opening output file
	*/	
	
	FILE *f3 = fopen(argv[3], "w+");
	if (f3 == NULL)
	{
		printf("Error creating output file!");
		exit(1);
	}
		
	/*
	Take input for graph
	*/
	
	int G[MAX][MAX] = {{EMPTY}, {EMPTY}};							//Adjacency matrix of the graph G
	int D[MAX][MAX] = {{EMPTY}, {EMPTY}};							//Matrix containing the distance between two nodes in grapgh G
		
	FILE *f2 = fopen(argv[2], "r");
	if(!f2)
	{	
		printf("\nCould not read from the graph file!");
		exit(-1);
	}
	
	/*
	Populating adjacency matrix from edge list in input file
	*/
	
	int vertices;
	fscanf(f2, "%d", &vertices);
	
	int i, j;
	
	while(fscanf(f2, "%d %d", &i, &j) > 0)
	{
		if(i >= vertices || j >= vertices || i == j)
		{
			printf("\nInvalid edge in Graph File...");
			exit(-1);
		}
		
		G[i][j] = 1;
	}
		
	for(int i=0; i<MAX; i++)
		for(int j=0; j<MAX; j++)
			if(i < vertices && j < vertices)
			{
				if(G[i][j] == EMPTY)
					G[i][j] = 0;
			}
			else
			{
				G[i][j] = EMPTY;
			}
	
	/*
	Reading from input file and populating the array with problem values
	*/
	
	fscanf(f1, "%d", &s0);
	if(feof(f1))
	{
		printf("\nProblem file is empty. Please provide all constraint and demand values...");
		exit(-1);
	}
	
	fscanf(f1, "%d", &s1);
	if(feof(f1))
	{
		printf("\nProblem file does not specify all interference constraints...");
		exit(-1);
	}	
	
	fscanf(f1, "%d", &s2);
	if(feof(f1))
	{
		printf("\nProblem file does not specify all interference constraints...");
		exit(-1);
	}	
		
	for(j=0; j<vertices; j++)
	{
		fscanf(f1, "%d", &XW[1][j]);
		if(feof(f1))
		{
			XW[1][j] = EMPTY;
			
			for(int k = j; k<MAX; k++)
				XW[0][k] = EMPTY;
				
			break;
		}
		
		XW[0][j] = j;
	}
	
	if(j != vertices)
	{
		printf("\nProblem file does not specify demands for all nodes...");
		exit(-1);
	}
	
	if(DEBUG) {
		printf("\ns0: %d\ns1: %d\ns2: %d\n\n\nXW:-\n\n", s0, s1, s2);
		displayXW(XW);
	}
	
	if(DEBUG) {			
		printf("\n\n----------------ADJACENCY MATRIX----------------\n\n");
		displayGraph(G);
	}
	
	calculateDistance(G, D);
	calculateC(D);
	
	if(DEBUG) {	
		printf("\n\n---------------SHORTEST DISTANCE----------------\n\n");
		displayGraph(D);
		
		printf("\n\n--------FREQUENCY SEPARATION CONSTRAINTS---------\n\n");
		displayGraph(C);
	}
	
	assignFrequency(XW, G, f3);
}

void calculateDistance(int G[MAX][MAX], int D[MAX][MAX]) {
	int max = MAX;
	
	for(int j = 0; j<MAX; j++)
		if(G[1][j] == EMPTY)
		{
			max = j;
			break;
		}
			
	memcpy(D, G, MAX*MAX*sizeof(int));
		
	for(int i = 0; i<max; i++)
		for(int j = 0; j<max; j++)
		{
			if(i == j)
				D[i][i] = 0;
			else if(G[i][j] == 0)
				D[i][j] = INFINITY;
		}

	for(int k = 0; k<max; k++)
	{
		int Dk[MAX][MAX] = {{EMPTY}, {EMPTY}};
		for(int i = 0; i<max; i++)
			for(int j = 0; j<max; j++)
				Dk[i][j] = min(D[i][j], D[i][k]+D[k][j]);
		memcpy(D, Dk, MAX*MAX*sizeof(int));
	}
}

void calculateC(int D[MAX][MAX]) {
	int max = MAX;
	
	for(int j = 0; j<MAX; j++)
		if(D[1][j] == EMPTY)
		{
			max = j;
			break;
		}
		
	for(int i = 0; i<max; i++)
		for(int j = 0; j< max; j++)
		{
			if(D[i][j] == 0)
				C[i][j] = s0;
			else if(D[i][j] == 1)
				C[i][j] = s1;
			else if(D[i][j] == 2)
				C[i][j] = s2;
			else
				C[i][j] = 0;
		}
}

void displayXW(int XW[2][MAX]) {
		
	for(int i=0; i<2; i++)
	{		
		for(int j=0; j<MAX; j++)
		{
			if(XW[i][j] == EMPTY)
				break;
				
			printf("%d\t",XW[i][j]);
		}		
		printf("\n");
	}
}

void displayGraph(int G[MAX][MAX]) {
	
	for(int i=0; i<MAX; i++)
	{
		if(G[i][0] == EMPTY)
			break;
			
		for(int j=0; j<MAX; j++)
		{
			if(G[i][j] == EMPTY)
				break;
				
			printf("%d\t", G[i][j]);
		}		
		printf("\n");
	}
	
	printf("\n");
}

void assignFrequency (int XW[2][MAX], int G[MAX][MAX], FILE *f3) {
	int origin = 0;		
	int QphiQ[2][MAX] = {{EMPTY}, {EMPTY}};
	
	int XW_dash[2][MAX], XR_dash[2][MAX];
	int phase = 1;
	
	do {
	
		if(DEBUG)
			printf("\n\n------------PHASE %d--------------",phase);
		
		fprintf(f3, "Phase %d\n\n", phase);
		phase++;
		
		memcpy(XW_dash, XW, 2*MAX*sizeof(int));
		
		/*
		Step 2. Find X' and W'
		*/
		
		sort(XW, XW_dash, G);
		
		if(DEBUG) {
			printf("\nX'W' :-\n\n");
			displayXW(XW_dash);
		}
		
		/*
		Step 3. Find increment
		*/
		
		int YphiY[2][MAX] = {{EMPTY}, {EMPTY}};
		
		int i = 0, j = 0;
		int maxDemand = XW_dash[1][i];	
		
		while(XW_dash[0][i] != EMPTY && XW_dash[1][i] == maxDemand) {
			YphiY[0][j] = XW_dash[0][i];
			YphiY[1][j] = EMPTY;
			i++;
			j++;
		}
		
		for( ; j<MAX; j++)
		{
			YphiY[0][j] = EMPTY;
			YphiY[1][j] = EMPTY;
		}
		
		if(DEBUG) {
			printf("\nYphiY :-\n\n");
			displayXW(YphiY);
		}
		
		int increment;
		increment = findIncrement(YphiY, QphiQ, origin);
		
		if(DEBUG)
			printf("\n\nIncrement = %d", increment);
		
		/*
		Step 4. Find Assignment
		*/
		
		int VphiV[2][MAX][MAXFREQ], V_dash[MAX], XphiX_dash[2][MAX];
		for(int j=0; j<MAX; j++)
		{
			XphiX_dash[0][j] = XW_dash[0][j];
			XphiX_dash[1][j] = EMPTY;
		}
			
		findAssignment(XphiX_dash, QphiQ, origin, increment, VphiV, V_dash);
		
		if(DEBUG)
		{
			printf("\n\nVphiV:-\n\n");
			for(int i = 0; i < 2; i++)
			{	
				for(int j = 0; j < MAX; j++)
				{
					if(VphiV[i][j][0] == EMPTY)
						break;
				
					printf("%d\t", VphiV[i][j][0]);
				}
			
				printf("\n");
			}
		}
		
		if(DEBUG)
		{
			printf("\n\nV_dash:-\n\n");
			
			for(int i = 0; i<MAX; i++)
			{
				if(V_dash[i] == EMPTY)
					break;
				printf("%d\t", V_dash[i]);
			}
		}
		
		/*
		Step 5. Assign multiple frequencies
		*/
		
		int gamMax, gamMax_dash;
		
		for(int i = 0; i<MAX; i++)
			if(XW_dash[0][i] == VphiV[0][0][0])
			{
				gamMax = XW_dash[1][i];
				break;
			}
			
		for(int i = 0; i<MAX; i++)
			if(XW_dash[0][i] == V_dash[0])
			{
				gamMax_dash = XW_dash[1][i];
				break;
			}
		
		int D[2][MAX], wv;
		
		for(int i=0; i<MAX; i++)
		{
			D[0][i] = EMPTY;
			D[1][i] = EMPTY;
		}
		
		if(DEBUG)
			printf("\ngammaMax = %d, gammaMax' = %d\n", gamMax, gamMax_dash);
		
		for(int i=0; i<MAX; i++)
		{
			if(VphiV[0][i][0] == EMPTY)
				break;
				
			D[0][i] = VphiV[0][i][0];
			
			for(int j = 0; j<MAX; j++)
				if(XW[0][j] == VphiV[0][i][0])
				{					
					wv = XW[1][j];
				}
				
			if(wv >= gamMax - gamMax_dash)
				D[1][i] = gamMax - gamMax_dash;
			else
				D[1][i] = wv;		
		}
		
		if(DEBUG) {		
			printf("\n\nD:-\n\n");
			displayXW(D);
		}
		
		int Vcount;
		for(int i=0; i<MAX; i++)
			if(VphiV[0][i][0] == EMPTY)
			{
				Vcount = i;
				break;
			}
		
		for(int i=0; i<Vcount; i++)
		{
			int dv = D[1][i];
			for(int j=1; j<dv; j++)
				VphiV[1][i][j] = VphiV[1][i][0] + j*increment;
				
			for(int j=dv+1;j<MAXFREQ; j++)
				VphiV[1][i][j] = EMPTY;
		}
		
		if(DEBUG) 
			printf("\n\nAssignment:-\n\n");
		
		for(int j=0; j<Vcount; j++)
		{		
			int dv = D[1][j];
			if(DEBUG) printf("%d:\t", VphiV[0][j][0]);
			fprintf(f3, "%d:\t", VphiV[0][j][0]);
			for(int k=0; k<=dv; k++)
			{
				if(VphiV[1][j][k] == EMPTY)
					break;
				if(DEBUG) printf("%d\t", VphiV[1][j][k]);
				fprintf(f3, "%d\t", VphiV[1][j][k]);
			}
			if(DEBUG) printf("\n");
			fprintf(f3, "\n");
		}
		
		fprintf(f3, "\n");
		
		/*
		Step 6. Update
		*/
		
		int max = MAX;
		
		for(int j = 0; j<MAX; j++)
			if(XW_dash[1][j] == EMPTY)
			{
				max = j;
				break;
			}
		
		for(int j=0; j<max; j++)
		{
			XR_dash[0][j] = XW_dash[0][j];
			wv = XW_dash[1][j];
				
			if(belongsTo(V_dash, XR_dash[0][j]))
				XR_dash[1][j] = wv;			
			else if(wv >= (gamMax - gamMax_dash))
				XR_dash[1][j] = wv - (gamMax - gamMax_dash);
			else
				XR_dash[1][j] = 0;
		}
		
		for(int j=max; j<MAX; j++)
			XR_dash[0][j] = XR_dash[1][j] = EMPTY;
		
		for(int j=0; j<MAX; j++)
		{
			QphiQ[0][j] = EMPTY;
			QphiQ[1][j] = EMPTY;
		}
			
		j = 0;
		int assigned;
		
		for(int b=0; b<MAX; b++)
		{
			assigned = 0;
			for(int c=0; c<MAXFREQ; c++)
				if(VphiV[1][b][c] != EMPTY)
					assigned++;
				else break;
			
			for(int d=0; d<MAX; d++)
				if(XW_dash[0][d] == VphiV[0][b][0])
				{
					if(XW_dash[1][d] - assigned > 0)
					{
						QphiQ[0][j] = VphiV[0][b][0];
						QphiQ[1][j] = last(VphiV, j);
						j++;
					}
					break;
				}
		}
		
		if(DEBUG) {
			printf("\n\nXR_dash :-\n\n");
			displayXW(XR_dash);
		
			printf("\n\nQphiQ :-\n\n");
			displayXW(QphiQ);
		}
		
		origin = maxfr(QphiQ)+1;
		if(DEBUG)
			printf("\n\nOrigin next:%d", origin);
		
		for(int l = 0; l<MAX; l++)
		{
			XW[0][l] = EMPTY;
			XW[1][l] = EMPTY;
		}
		
		int k = 0;
		for(int l = 0; l<MAX; l++)
		{
			if(XR_dash[1][l] == EMPTY)
				break;
				
			if(XR_dash[1][l] > 0)
			{
				XW[0][k] = XR_dash[0][l];
				XW[1][k] = XR_dash[1][l];
				k++;
			}
		}
		
		for( ; k<MAX; k++)
		{
			XW[0][k] = EMPTY;
			XW[1][k] = EMPTY;
		}
		
		if(DEBUG) {
			printf("\n\nXW :-\n\n");
			displayXW(XW);
		}
		
	} while(nonzero(XR_dash));
}

int last(int V[2][MAX][MAXFREQ], int j) {
	int lastn = 0;
	
	for(int k=0; k<MAXFREQ; k++)
	{
		if(V[1][j][k] == EMPTY)
		{
			lastn = V[1][j][k-1];
			break;
		}
	}
	
	return lastn;
}

int maxfr(int V[2][MAX]) {
	int max = V[1][0];
	
	for(int i=0; i<MAX; i++)
	{
		if(V[1][i] == EMPTY)
			break;
		if(V[1][i] > max)
			max = V[1][i];
	}
	
	return max;
}

int nonzero(int V[2][MAX]) {
	for(int i=0; i<MAX; i++)
	{
		if(V[1][i] == EMPTY)
			break;
		if(V[1][i] != 0)
			return 1;
	}
	
	return 0;
}

int belongsTo(int V[MAX], int j) {
	for(int i = 0; i<MAX; i++)
	{
		if(V[i] == EMPTY) 
			return 0;
		if(V[i] == j)
			return 1;
	}
	
	return 0;
}

void sort (int XW[2][MAX], int XW_dash[2][MAX], int G[MAX][MAX]) {
	
	/*
	Step 2.1. Bubble-sort algorithm to sort the nodes according to the descending order of demands
	*/
	
	int flag;
	int max = MAX;
	
	for(int j = 0; j<MAX; j++)
		if(XW_dash[1][j] == EMPTY)
		{
			max = j;
			break;
		}
			
	for(int i = 0; i<max; i++)
	{
		flag = 0;
		
		for(int j = 0; j<max-i-1; j++)
		{
			if(XW_dash[1][j] < XW_dash[1][j+1])
			{
				flag++;
				SWAP(XW_dash[0][j], XW_dash[0][j+1]);
				SWAP(XW_dash[1][j], XW_dash[1][j+1]);
			}
		}
		
		if(flag == 0)
				break;
	}
		
	/*
	Sort tied nodes according to the descending order of the sums of the demands of their adjacent nodes andif even that is a tie, then sort according to the
	increasing order of their node index numbers,
	*/
	
	for(int j = 0; j<max; j++)
	{
		int start = j, end = j;
		for(int k = j+1; k<max; k++)
		{
			if(XW_dash[1][k] == XW_dash[1][j])
				end = k;
		}
		
		int length = end - start + 1;
		if(length == 0)
			continue;
			
		int adjacentDemandSums[length];
		for(int l = 0; l<length; l++)
		{
			adjacentDemandSums[l] = calculateAdjacentDemandSum(XW_dash[0][start+l], XW, G);
		}
		
		for(int i = 0; i<length; i++)
		{
			flag = 0;
			for(int l = 0; l<length-i-1; l++)
			{
				//Checking for descending order of adjacent demands
				if(adjacentDemandSums[l] < adjacentDemandSums[l+1])
				{
					flag++;
					SWAP(adjacentDemandSums[l], adjacentDemandSums[l+1]);
					SWAP(XW_dash[0][start+l], XW_dash[0][start+l+1]);
					SWAP(XW_dash[1][start+l], XW_dash[1][start+l+1]);
				}
				
				//Checking for increasing order of node indices if adjacent demands are tied
				else if(adjacentDemandSums[l] == adjacentDemandSums[l+1] && XW_dash[0][start+l] > XW_dash[0][start+l+1])
				{
					flag++;
					SWAP(XW_dash[0][start+l], XW_dash[0][start+l+1]);
					SWAP(XW_dash[1][start+l], XW_dash[1][start+l+1]);
				}
			}
			
			if(flag == 0)
				break;
		}
		
		j = end;
	}		
}

int calculateAdjacentDemandSum (int node, int XW[MAX][MAX], int G[MAX][MAX]) {
	
	int sum = 0;
	
	for(int j = 0; j < MAX; j++)
		if(G[node][j] == 1)
		{
			for(int i=0; i<MAX; i++)
				if(XW[0][i] == j)
				{
					sum+=XW[1][i];
					break;
				}
		}
		
	return sum;
}

int findIncrement(int YphiY[2][MAX], int QphiQ[2][MAX], int origin) {
	int increment = s0;
	int m = 0, r = 0, flag, found;
	
	for(int i=0; i < MAX; i++)
	{
		if(YphiY[0][i] != EMPTY)
			m++;
		else
			break;
	}	
	
	for(int i=0; i < MAX; i++)
	{
		if(QphiQ[0][i] != EMPTY)
			r++;
		else
			break;
	}
		
	for(int i=0; i<m; i++)
	{
		found = 0;		
		
		for(int x=origin; x < origin+increment; x++)
		{
			int phi_y_i = x;
			flag = 1;
			
			for(int k = 0; k < r; k++)
				if(abs(phi_y_i - QphiQ[1][k]) < C[YphiY[0][i]][QphiQ[0][k]])
					flag = 0;
					
			for(int j = 0; j<i; j++)
				if(YphiY[1][j] != EMPTY && abs(phi_y_i - YphiY[1][j]) < C[YphiY[0][i]][YphiY[0][j]])
					flag = 0;
					
			for(int j = 0; j<i; j++)
				if(YphiY[1][j] != EMPTY && (increment - abs(phi_y_i - YphiY[1][j])) < C[YphiY[0][i]][YphiY[0][j]])
					flag = 0;
					
			if(flag)
			{
				found = 1;
				YphiY[1][i] = phi_y_i;
				break;
			}
		}
		
		if(!found)
		{
			increment++;
			i--;
		}
	}
	
	return increment;
}

void findAssignment (int XphiX_dash[2][MAX], int QphiQ[MAX][MAX], int origin, int increment, int VphiV[2][MAX][MAXFREQ], int V_dash[MAX]) {

	for(int i = 0; i < MAX; i++)
	{
		for(int j = 0; j < 2; j++)
			for(int k = 0; k < MAXFREQ; k++)
				VphiV[j][i][k] = EMPTY;
		V_dash[i] = EMPTY;
	}
	
	int t = MAX, r = 0, VphiVcounter = 0, V_dashcounter = 0, flag, found;
	
	for(int j = 0; j<MAX; j++)
		if(XphiX_dash[0][j] == EMPTY)
		{
			t = j;
			break;
		}
	
	for(int i=0; i < MAX; i++)
	{
		if(QphiQ[0][i] != EMPTY)
			r++;
		else
			break;
	}
	
	int phi_x_i_dash;
	
	for(int i = 0; i<t; i++)
	{				
		found = 0;
		
		for(int x=origin; x < origin+increment; x++)
		{
			phi_x_i_dash = x;
			flag = 1;
			
			for(int k = 0; k < r; k++)
				if(abs(phi_x_i_dash - QphiQ[1][k]) < C[XphiX_dash[0][i]][QphiQ[0][k]])
					flag = 0;
					
			for(int j = 0; j<i; j++)
				if(XphiX_dash[1][j] != EMPTY && abs(phi_x_i_dash - XphiX_dash[1][j]) < C[XphiX_dash[0][i]][XphiX_dash[0][j]])
					flag = 0;
			
			for(int j = 0; j<i; j++)
				if(XphiX_dash[1][j] != EMPTY && (increment - abs(phi_x_i_dash - XphiX_dash[1][j])) < C[XphiX_dash[0][i]][XphiX_dash[0][j]])
					flag = 0;
				
			if(flag)
			{
				found = 1;
				break;
			}
		}
		
		if(found)
		{
			XphiX_dash[1][i] = phi_x_i_dash;
			VphiV[0][VphiVcounter][0] = XphiX_dash[0][i];
			VphiV[1][VphiVcounter][0] = XphiX_dash[1][i];
			VphiVcounter++;
		}
		else
		{
			V_dash[V_dashcounter] = XphiX_dash[0][i]; 
			V_dashcounter++;
		}
	}
}
