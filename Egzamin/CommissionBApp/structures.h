#ifndef STRUCTURES_H
#define STRUCTURES_H // definicje struktur danych

#include "utils.h"
#include <sstream>
#include <vector>

struct Student // struktura studenta
{
	int id; // id studenta
	int direction; // kierunek studi�w studenta
	bool practic_pass; // czy zdany egzamin praktyczny
	bool theoric_pass; // czy zdany egzamin teoretyczny
	float practic_grade; // ocena za egzamin praktyczny
	float theoric_grade; // ocena za egzamin teoretyczny

	Student() : id(0), direction(0), practic_pass(false), theoric_pass(false), practic_grade(0.0), theoric_grade(0.0) {} // konstruktor domy�lny
};

struct Exam_result // struktura listy wynik�w egzaminu
{
	int student_id; // id studenta zdaj�cego egzamin
	float final_grade; // finalna ocena za ca�y egzamin
	bool passed; // czy zdany

	Exam_result() : student_id(0), final_grade(0.0), passed(false) {} // konstruktor domy�lny
};

#endif
