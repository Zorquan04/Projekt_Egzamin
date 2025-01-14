#ifndef EXAM_LOGIC_H
#define EXAM_LOGIC_H

#include "structures.h"
#include <unistd.h>
#include <cstdlib>
#include <ctime>
#include <cmath>

void generate_students(int, int, int, vector<Student>&); // losowanie studentów na podstawie liczby kierunków oraz liczby studentów na ka¿dym z kierunków

void simulate_answers(Student&); // symulacja odpowiedzi studenta oraz ocenianie

float calculate_final_grade(const Student&); // wyliczanie œredniej oceny z obu czêœci egzaminu

#endif
