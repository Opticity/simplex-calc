/******************************************************************************************************************************************************

MATRIX FORMAT:

Assume no. of variables = 2
	   no. of constraints = 3

No. of slack variables = 3

Initial table:

		[0]	[1]	[2]	[3]	[4]
col:	x1	x2	x3	x4	x5	 Sol IR
max:	-3	-2 	 0	 0	 0	 0	 0		[0]
x3:		2 	 1   1	 0	 0	 18	 9		[1]
x4:		2 	 3   0	 1	 0	 42	 21		[2]
x5:	   [3] 	 1   0	 0	 1	 24	 8		[3]

Variable contents:

m_Basic[i] = [3] 		m_Matrix[i][j] = [-3 -2  0  0  0]
			 [4]  			  	  	     [ 2  1  1  0  0]
			 [5]  	  		  	  	     [ 2  3  0  1  0]
  	  	  	    						 [ 3  1  0  0  1]
										 
m_Solution[i] = [ 0]	m_Ratio[i] = [ 0]
				[18]				 [ 9]
				[42]				 [21]
				[24]				 [ 8]
				
m_Pivot[i] = [3] -- 4th row
			 [0] -- 1st column
 
******************************************************************************************************************************************************/

#include "Tableau.h"
#include "MiscFunc.h"

#include <math.h>
#include <stdlib.h>
#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
using namespace std;

#define ROW 0
#define COL 1

Tableau::Tableau(int m, int n, bool Minimize)
{

	m_Row = m;
	isMinimize = Minimize;
	m_Column = n+1;
	
	m_SlackNum = m_Column - 1; // starting index of slack variables, NOT number of slack variables
	m_Counter = 0,
	m_Pivot[0] = m_Pivot[1] = 0;
	m_Solution[0] = 0;
	m_Ratio[0] = 0;

	for (int i=0; i<=m; i++)
	{
		for (int j=0; j<=m+n; j++)
		{
			m_Matrix[i][j] = 0;
		}
	}

	isMultiple = false;
	isUnbounded = false;
}

void Tableau::setTable(double (&matrix)[200][200], double solution[200])
{

	int i,j,k;
	for(i=0;i<=m_Row;i++)
	{
		for(j=0;j<=m_Column;j++)
		{
			m_Matrix[i][j] = matrix[i][j];
		}
		if(i!=0) m_Solution[i] = solution[i];
	}
	
	
	k = m_SlackNum+1;	 

	if(isMinimize == false) 
	   for(j=0;j<=m_Column;j++)
			m_Matrix[0][j] *= -1;
	
	addSlackVar();
	for(i=1; i<=m_Row; i++)
	{
		m_Basic[i-1] = k; // initialize m_Basic array with indices of slack variables
		k++;
	}
}

void Tableau::addSlackVar()
{
	int i, j;
	
	for(i=1; i<=m_Row; i++)
	{
		for(j=m_SlackNum; j<m_SlackNum+m_Row; j++)
		{
			if((i+m_SlackNum-1)==j) m_Matrix[i][j] = 1; // create the identity matrix in m_Matrix
			else m_Matrix[i][j] = 0;
		}
	}
	m_Column += m_Row;
}

void Tableau::Initialize()
{
	printStandardForm();
	findPivot();
	printTable("at initialization");
	if(isUnbounded == true)
	{
		m_Output << endl << "Unbounded solution! This problem cannot be solved." << endl;
		m_Output << "This occurs when the intercept ratio column is all negative." << endl;
		m_Output << "There may be a missing constraint. Please reenter your problem." << endl;	   	   
	}
	else
	{
		printVar();
    	Optimize();
		if(isUnbounded == false) showResult();
	}
}

void Tableau::findPivot()
{
	// Steps:
	// 1. Find most negative column in objective row. That is the entering variable column.
	// 2. Find smallest non-negative intercept ratio. That is the leaving variable row.
	// 3. The intercept of the entering variable column and the leaving variable row is the pivot.
	
	// Step 1. Find most negative column in objective row
	
	int i, j;
	int colIndex = 0;
	int rowIndex = 0;
	int counter = 0;
	double temp;
	
	if(equal(m_Matrix[0][0],0.0)==0.0) // has to be done in case of negative 0 (yes, it happens)
		temp = 0.0;
	else
		temp = m_Matrix[0][0]; // temp = 1st element of objective function row
	
	for(j=1; j<m_Column-1; j++)
	{
		if(equal(m_Matrix[0][j],0.0)<temp) // if there are any elements in row 0 that is smaller than temp, replace temp with that value
		{
			temp = equal(m_Matrix[0][j],0.0);
			colIndex = j;
		}
	}

	temp = 99999999;
	
	if(isMultiple == true) colIndex = m_Pivot[COL];
	

	// Step 2. Calculate intercept ratio
	
	for(i=1; i<=m_Row; i++) // update ratio column only for rows 1~n (ratio is not calculated for row 0)
	{
		m_Ratio[i]= m_Solution[i]/m_Matrix[i][colIndex]; // calculate intercept ratio for current row
		
		if(m_Ratio[i] > 0 && m_Ratio[i] < temp) // if ratio for current row more than 0 and smaller than temp, update temp
		{
			temp = m_Ratio[i];
			rowIndex = i; // the current row is the pivot row (repeat and update again if the next rows have smaller ratio)
		}
		else if(m_Ratio[i] < 0 || isinf(m_Ratio[i]) || isnan(m_Ratio[i])) counter++;
	}
	
	if(counter == m_Row) isUnbounded = true;
	
	// Step 3. Pivot identified
	
	m_Pivot[ROW] = rowIndex;
	m_Pivot[COL] = colIndex;
}

void Tableau::swapBasic()
{
	m_Basic[m_Pivot[ROW]-1] = m_Pivot[COL]+1; // swap m_Basic index with pivot column
}

void Tableau::rowOperation()
{
	int i,j;
	double mult;

	// Steps:
	// 1. Divide pivot row by pivot value
	// 2. Perform row operation on other rows using the following algorithm:
	//	  	  	  
	//			matrix[i][j] = matrix[i][j] - matrix[i][pivotColumn]*matrix[pivotRow][j]
	//

	// Step 1: Divide pivot row by pivot value

	double pivotVal = m_Matrix[m_Pivot[ROW]][m_Pivot[COL]];
	for(j=0; j<m_Column; j++)
	{
		m_Matrix[m_Pivot[ROW]][j] /= pivotVal;
	}
	m_Solution[m_Pivot[ROW]] /= pivotVal;
	
	// Step 2: Perform row operation on other rows using new pivot row
	
	for(i=0; i<=m_Row; i++)
	{
		if(i == m_Pivot[ROW]) continue; //skip pivot row
		else
		{
			mult = m_Matrix[i][m_Pivot[COL]];			
			for(j=0; j<m_Column; j++)
			{
				m_Matrix[i][j] -= (mult*m_Matrix[m_Pivot[ROW]][j]);
			}
			m_Solution[i] -= (mult*m_Solution[m_Pivot[ROW]]);
		}
	
	}
}

void Tableau::Optimize()
{	 
	// Concept: Iteration must be repeated until the variables in the objective function row are non-negative.

	int j;
	
	bool isOptimal = false;
	int nCounter = 0;

	do
	{
		swapBasic();
		rowOperation();
		findPivot();
		printTable(" ");
		for(j=0; j<m_Column; j++) // check row 0 of m_Matrix for negative elements
			if(equal(m_Matrix[0][j],0.0)<0) nCounter++;
		
		if(nCounter==0) isOptimal = true; // if there are none, stop calculation
		if(isUnbounded == true) break;
		if(isOptimal == false) printVar();
		nCounter = 0;	 	 
	}
	while(isOptimal == false); // loop while there are still negative elements in row 0
	
	checkMultiple();
	if(isMultiple == true)
	{
		showResult();
		m_Output << "Multiple solutions available!" << endl;
		m_Output << "Tableau will be revised. (Intercept ratio recalculated)" << endl;
		printTable("revised");
		printVar();
		swapBasic();
		rowOperation();
		findPivot();
		printTable("after one more iteration");
	}
	
	if(isUnbounded == true)
	{
		m_Output << endl << "Unbounded solution! This problem cannot be solved." << endl;
		m_Output << "This occurs when the intercept ratio column is all negative." << endl;
		m_Output << "There may be a missing constraint. Please reenter your problem." << endl;
	}
}

void Tableau::checkMultiple()
{
	// Special case: Multiple/infinite solutions
	// Characterized by having a non-basic variable having a coefficient of zero in Row 0 when optimal solution is found
	// To discover the other solution, repeat iteration with the non-basic variable as the entering variable
	
	// Step 1: Check Row 0 for non-basic variables with coefficient of 0
	
	int i,j;
	bool isNotBasic = true;
	
	for(j=0; j<=m_SlackNum; j++)
	{
		for(i=0; i<=m_Row; i++)
		{
			if(m_Basic[i]==j+1) 
			{
				isNotBasic = false;
				break;
			}
			else
				isNotBasic = true;
		}
		if(isNotBasic==true)
		{
			if(equal(m_Matrix[0][j],0.0)==0)
			{
				isMultiple = true;
	
				m_Pivot[COL] = j;
				findPivot();
				
				m_Pivot[COL] = j;
			}
		}
	}
}

void Tableau::printTable(string mes)
{	 
	int i,j;
	
	if(mes.compare(" ")==0)
		m_Output << endl << "Tableau after " << ++m_Counter << " iteration(s)" << endl;
	else
	{
		m_Output << endl << "Tableau " << mes << endl;
	}
	
	m_Output << drawLine(75);
	m_Output << "col:";
	
	for(j=1; j<m_Column; j++)
	{
		m_Output << "\t x" << j;
	}
	m_Output << "\t Sol.\t IR" << endl;
	
	for(i=0; i<=m_Row; i++)
	{
		if(i==0)	m_Output << "z:";
		else		m_Output << "x" << m_Basic[i-1] << ":";

		for(j=0; j<m_Column-1;j++)
		{
			if(equal(m_Matrix[i][j],0.0)==0)
				m_Output << "\t 0.00";
			else m_Output << fixed << setprecision(2) << "\t " << m_Matrix[i][j];
		}
		
		m_Output << "\t " << m_Solution[i];
		m_Output << "\t " << m_Ratio[i] << endl;
		
	}
	m_Output << drawLine(75);
}

void Tableau::printVar()
{
	m_Output << endl << "Entering variable : x" << m_Pivot[COL]+1;
	m_Output << endl << "Leaving variable  : x" << m_Basic[m_Pivot[ROW]-1] << endl;
}

void Tableau::printStandardForm()
{
	int i, j;

	for(i=0; i<=m_Row; i++)
	{
		if(i==0) 
		{
			m_Output << "z";
			if(m_Matrix[0][1] < 0) m_Output << " - " << fixed << setprecision(3);
			else m_Output << " + ";
		}
		else m_Output << "    ";
		
		for(j=0; j<m_Column-1;j++)
		{
			if(m_Matrix[i][j] < 0)
			{
				if(j!=0) m_Output << " - " << -m_Matrix[i][j] << " x" << j+1;
				else m_Output << -m_Matrix[i][j] << " x" << j+1;
			}
			else
			{
				if(j!=0) m_Output << " + " << m_Matrix[i][j] << " x" << j+1;
				else  m_Output << m_Matrix[i][j] << " x" << j+1;
			}
		}
		
		m_Output << " = " << m_Solution[i] << endl;
	}
}

void Tableau::showResult()
{
	int i, j;
	bool isBasic = false;
	int solRow = 0;
	if(isMinimize == false) m_Output << endl << "Optimal solution = " << fixed << setprecision(6) << m_Solution[0] << endl;
	else m_Output << endl << "Optimal solution = " << fixed << setprecision(6) << -m_Solution[0] << endl;
	m_Output << "Variable values: " << endl;
	for(i=1; i<=m_SlackNum; i++)
	{
		m_Output << "x" << i << " = ";
		for(j=0; j<=m_SlackNum; j++)
		{
			if(i==m_Basic[j]) 
			{
				isBasic = true;
				solRow = j+1;
				break;
			}
		}
		if(isBasic == true)
			m_Output << fixed << setprecision(6) << m_Solution[solRow] << endl;
		else
			m_Output << '0' << endl;
		isBasic = false;
	}
	m_Output << endl;
}	 

string Tableau::getTable()
{
	return m_Output.str();
}

