#include "Tableau.h"

#include <iostream>
#include <string>
using namespace std;

int main()
{

	string output;
	
	double matrix[200][200]	 = 
	 {{5.0, 3.0, 1.0},
	  {1.0, 1.0, 1.0},
	  {5.0, 3.0, 6.0}};

/*	    double matrix[200][200] =	    
	  {{4.0, -10.0, -20.0},
	   {3.0, 4.0, 5.0},
	   {2.0, 1.0, 0.0},
	   {2.0, 0.0, 3.0}};*/
	  
/*	  double matrix[200][200] =
	{{2, -3},
	 {1, 1},
	 {1, -1}};*/
	
	double solution[200] = {0, 6, 15};
	int con = 2;
	int var = 3;
	
	Tableau tab(con,var,false);
	tab.setTable(matrix,solution);
	tab.printStandardForm();
	tab.findPivot();
	tab.printTable("at initialization");
	tab.printVar();
	
    tab.Optimize();
	tab.showResult();
	
	output = tab.getTable();
	
	cout << output;
	return 0;
}

