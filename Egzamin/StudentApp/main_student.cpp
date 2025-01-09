#include <cstdlib>
#include <ctime>
#include "utils.h"
#include "structures.h"
#include "constants.h"

int main()
{
	srand(time(nullptr)); // inicjalizacja generatora losowego
	int student_id = rand() % 1000 + 1;

	cout << "Student o ID = " << student_id << " gotowy na egzamin..." << endl;

	// obs³uga komunikacji z dziekanem i komisj¹

	return 0;
}
