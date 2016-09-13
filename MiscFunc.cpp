#include "MiscFunc.h"
#include <iostream>
#include <math.h>
#include <string>
#include <sstream>
using namespace std;

static const double epsilon   = 1.0e-8;
double equal(double a, double b) 
{
	if(fabs(a-b)>=epsilon) return a;
	else return b;
}

string drawLine(int length)
{
	ostringstream oss;
	for(int i=0; i<length; i++)
	{
		oss << "-";
	}
	oss << endl;
	
	return oss.str();
}

