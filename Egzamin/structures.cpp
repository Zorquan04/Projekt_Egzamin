#include "structures.h"

ostream& operator<<(ostream& os, const Student& student) // przeci¹¿enie operatora << dla struktury Student
{
	os << "Przybyl student " << "ID: " << student.id << ", Kierunek: " << student.direction
		<< ", Praktyczny zdany: " << (student.practic_pass ? "Tak" : "Nie");
	return os;
}
