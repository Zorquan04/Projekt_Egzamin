#ifndef STRUCTURES_H
#define STRUCTURES_H // definicje struktur danych

#include <string>
#include <vector>

struct Student // struktura studenta
{
	int id; // id studenta
	int direction; // kierunek studiów studenta
	bool practic_pass; // czy zdany egzamin praktyczny
	bool theoric_pass; // czy zdany egzamin teoretyczny
	float practic_grade; // ocena za egzamin praktyczny
	float theoric_grade; // ocena za egzamin teoretyczny
};

struct Exam_result // struktura do komunikacji z dziekanem
{
	int student_id; // id studenta zdaj¹cego egzamin
	float final_grade; // finalna ocena za ca³y egzamin
	bool passed; // czy zdany
};

#endif
