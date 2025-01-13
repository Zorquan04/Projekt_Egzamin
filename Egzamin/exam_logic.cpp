#include "exam_logic.h"

void generate_students(int num_directions, int min_students, int max_students, vector<Student>& students)
{
	srand(static_cast<unsigned int>(time(nullptr))); // inicjalizacja generatora losowego

	int student_id = 1; // globalny licznik dla id studentów (dla przydzielania unikalnego ID)
	for (int direction = 0; direction < num_directions; ++direction)
	{
		int num_students = rand() % (max_students - min_students + 1) + min_students; // generowanie liczby studentów odpowiadaj¹cej zakresowi podanemu w argumentach
		for (int i = 0; i < num_students; ++i)
		{
			Student student;
			student.id = student_id++; // przydzielenie unikalnego ID studenta
			student.direction = direction + 1; // przypisanie kierunku studiów
			student.practic_pass = false; // wstêpna ocena za praktykê studenta - pocz¹tkowo nie zdane
			student.theoric_pass = false; // wstêpna ocena za teoriê studenta - pocz¹tkowo nie zdane
			students.push_back(student); // dodanie studenta do listy
		}
	}
}

void simulate_answers(Student& student)
{
	student.practic_grade = static_cast<float>(rand() % 7 / 2.0 + 2.0); // losowanie oceny za odpowiedzi z Komisji A (praktyka)
	student.theoric_grade = static_cast<float>(rand() % 7 / 2.0 + 2.0); // losowanie oceny za odpowiedzi z Komisji B (teoria)

	student.practic_pass = (student.practic_grade > 2.0); // przydzielanie zaliczenia (true) jeœli ocena za praktyczny wy¿sza ni¿ 2.0
	student.theoric_pass = (student.theoric_grade > 2.0); // przydzielanie zaliczenia (true) jeœli ocena za teoretyczny wy¿sza ni¿ 2.0
}

float calculate_final_grade(const Student& student)
{
	if (student.practic_pass && student.theoric_pass) // jeœli teoria i praktyka zdana (jedno i drugie true)
	{
		float final_grade = static_cast<float>((student.practic_grade + student.theoric_grade) / 2.0); // finalna ocena jako œrednia ocen z teorii i praktyki

		final_grade = static_cast<float>(round(final_grade * 2) / 2.0); // zaokr¹glamy do 0.5 aby otrzymaæ ocenê rzeczywist¹

		return final_grade;
	}
	else
	{
		return 2.0; // jeœli teoria b¹dŸ praktyka nie zdana (false) zwracamy ocenê 2.0
	}
}
