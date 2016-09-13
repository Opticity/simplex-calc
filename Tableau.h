#ifndef __TABLEAU_H__
#define __TABLEAU_H__

#include <string>
#include <sstream>

class Tableau
{
private:
	int m_Row, m_Column, m_Counter, m_SlackNum;
	int m_Pivot[2];
	int m_Basic[200];
	double m_Matrix[200][200];
	double m_Solution[200];
	double m_Ratio[200];
	std::ostringstream m_Output;
	bool isMinimize, isMultiple, isUnbounded; 	    
public:
	Tableau(int m,int n, bool Minimize);
	void setTable(double (&matrix)[200][200],double solution[200]);
	void addSlackVar();
	void findPivot();
	void swapBasic();
	void rowOperation();
	void Optimize();
	void checkMultiple();
	void printTable(std::string mes);
	void printStandardForm();
	void printVar();
	void showResult();
	std::string getTable();
};

#endif

