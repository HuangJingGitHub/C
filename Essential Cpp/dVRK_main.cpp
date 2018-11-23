#include <iostream>
#include "Eigen/Dense"

using namespace std;
using namespace Eigen;

int main()
{
	MatrixXd m(2, 2);
	m(0, 0) = 1;
	m(0, 1) = 2;
	m(1, 0) = 3;
	m(1, 1) = 4;
	cout << m << endl;
	cout << "First test on Eigen library finished!\n";
}