// Jackson Burns
// 9-1-2023
// Lab 1 "Nuclear Blast Lab"
// Description: this code takes in 4 arguments on the commandline: inital blast power, attenuation factor,
// x coordinate of blast, and y coordinate of blast. It then takes in a list of names and coordinates from 
// standard input in the format: <x> <y> <name>. For every line thats read it calculates the distance of
// the person from the center of the blast and uses that to calculate how much radiation they take in
// sieverts (Sv). Each time it does this it prints it out formatted. It keeps going until it is out of lines
// to successfully read from.

#include <stdio.h>
#include <math.h>

struct Point
{
	int x;
	int y;
};

static double distance(const struct Point *p1, const struct Point *p2)
{
	double dist = 0.0;
	double tx = 0.0, ty = 0.0;

	// x2-x1 and y2-y1
	tx = (p2->x) - (p1->x);
	ty = (p2->y) - (p1->y);
	
	// (x2-x1)^2 and (y2-y1)^2
	tx *= tx;
	ty *= ty;

	// sqrt((x2-x1)^2 + (y2-y1)^2)
	dist = tx + ty;
	dist = sqrt(dist);

	return dist;
}

int main (int argc, char *argv[]) {

	// argv[1] = double initial explosion
	// argv[2] = double attenuation rate
	// argv[3] = x coordinate of blast
	// argv[4] = y coordiante of blast

	//Error checking number of command arguments
	if (argc != 5)
	{
		printf("Error, incorrect number of arguments\n");
		printf("%s <initial blast power> <attenuation factor> <x coord of blast> <y coord of blast>\n", argv[0]);
		return -1;
	}

	double power = 0.0;
	double atten = 0.0;

	//Error checking argv[1] while also storing the value if no error is found.
	//It does so by using sscanf() to parse the argument and cast it as a double
	//and since sscanf() returns the number of correctly read arguments it uses that
	//to determine if it was successful or not. If it fails it prints the error message
	//and returns -1 to exit the program.
	if (sscanf(argv[1], "%lf", &power) != 1)
	{
		printf("Error reading in initial blast power.\nPlease make sure it is numeric.\n");
		return -1;
	}

	//Error checking and storing argv[2]
	if (sscanf(argv[2], "%lf", &atten) != 1)
	{
		printf("Error reading in attenuation factor.\nPlease make sure it is numeric.\n");
		return -1;
	}

	//Error checking and storing blast coordinates
	struct Point blast;
	blast.x = 0;
	blast.y = 0;
	if (sscanf(argv[3], "%d", &(blast.x)) != 1) 
	{
		printf("Error reading in x coordinate.\nPlease make sure it is an integer.\n");
		return -1;
	}
	if (sscanf(argv[4], "%d", &(blast.y)) != 1)
	{
		printf("Error reading in y coordinate.\nPlease make sure it is an integer.\n");
		return -1;
	}
	
	//Taking in standard input to calculate the sieverts of each person given
	struct Point victim;
	victim.x = 0;
	victim.y = 0;
	char name[64];

	//Reads until it no longer reads a valid input. Also calculates and prints each loop
	//so it doesn't have to store anything
	while (scanf("%d %d %s", &(victim.x), &(victim.y), name) == 3)
	{
		//Calling distance function
		double dist = distance(&blast, &victim);

		//Calculates the amount of sieverts the current person receives 
		double sv = power * pow(atten, dist);

		//Prints formatted information
		printf("%-16s: %9.4lf Sv.\n", name, sv);
	}
}
