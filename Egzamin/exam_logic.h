#ifndef EXAM_LOGIC_H
#define EXAM_LOGIC_H

#include "structures.h"
#include <unistd.h>
#include <cstdlib>
#include <ctime>
#include <cmath>

void generate_students(int, int, int, vector<Student>&); // losowanie student�w na podstawie liczby kierunk�w oraz liczby student�w na ka�dym z kierunk�w

void simulate_answers(Student&); // symulacja odpowiedzi studenta oraz ocenianie

float calculate_final_grade(const Student&); // wyliczanie �redniej oceny z obu cz�ci egzaminu

#endif
