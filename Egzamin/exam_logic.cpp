#include "exam_logic.h"

void generate_students(int num_directions, int min_students, int max_students, vector<Student>& students)
{
	srand(static_cast<unsigned int>(time(nullptr))); // inicjalizacja generatora losowego

	int student_id = 1; // globalny licznik dla id student�w (dla przydzielania unikalnego ID)
	for (int direction = 0; direction < num_directions; ++direction)
	{
		int num_students = rand() % (max_students - min_students + 1) + min_students; // generowanie liczby student�w odpowiadaj�cej zakresowi podanemu w argumentach
		for (int i = 0; i < num_students; ++i)
		{
			Student student;
			student.id = student_id++; // przydzielenie unikalnego ID studenta
			student.direction = direction + 1; // przypisanie kierunku studi�w
			student.practic_pass = false; // wst�pna ocena za praktyk� studenta - pocz�tkowo nie zdane
			student.theoric_pass = false; // wst�pna ocena za teori� studenta - pocz�tkowo nie zdane
			students.push_back(student); // dodanie studenta do listy
		}
	}
}

void simulate_answers(Student& student)
{
	student.practic_grade = static_cast<float>(rand() % 7 / 2.0 + 2.0); // losowanie oceny za odpowiedzi z Komisji A (praktyka)
	student.theoric_grade = static_cast<float>(rand() % 7 / 2.0 + 2.0); // losowanie oceny za odpowiedzi z Komisji B (teoria)

	student.practic_pass = (student.practic_grade > 2.0); // przydzielanie zaliczenia (true) je�li ocena za praktyczny wy�sza ni� 2.0
	student.theoric_pass = (student.theoric_grade > 2.0); // przydzielanie zaliczenia (true) je�li ocena za teoretyczny wy�sza ni� 2.0
}

float calculate_final_grade(const Student& student)
{
	if (student.practic_pass && student.theoric_pass) // je�li teoria i praktyka zdana (jedno i drugie true)
	{
		float final_grade = static_cast<float>((student.practic_grade + student.theoric_grade) / 2.0); // finalna ocena jako �rednia ocen z teorii i praktyki

		final_grade = static_cast<float>(round(final_grade * 2) / 2.0); // zaokr�glamy do 0.5 aby otrzyma� ocen� rzeczywist�

		return final_grade;
	}
	else
	{
		return 2.0; // je�li teoria b�d� praktyka nie zdana (false) zwracamy ocen� 2.0
	}
}
