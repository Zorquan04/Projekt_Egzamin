#include "exam_logic.h"
#include "structures.h"
#include "constants.h"

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

			int random_pass = rand() % 100; // losowanie liczby z przedzia³u 0-99 w celu ustalenia, czy student powtarza egzamin i ma ju¿ zaliczon¹ praktykê
			if (random_pass >= 95) // 5% szans, ¿e student powtarza egzamin i ma ju¿ zdan¹ praktykê
			{
				student.practic_grade = 5.0;
				student.practic_pass = true;
			}
			students.push_back(student); // dodanie studenta do listy

			// symulacja losowego czasu pojawiania siê studentów
			int delay = rand() % 91 + 10; // losowy czas w milisekundach (10-100 ms)
			usleep(delay * 1000); // konwersja na mikrosekundy
			cout << students.back() << endl; // odwo³anie do ostatnio dodanego indeksu w wektorze w celu jego wyœwietlenia
		}
	}
}

void simulate_answers(Student& student)
{
	// Symulacja zadawania pytania przez komisjê
	int delay_chance = rand() % 100; // losowanie liczby z przedzia³u 0 - 99 w celu ustalenia, czy komisja siê spóŸni³a z pytaniem
	int question_delay = 0;
	if (delay_chance > 49) // 50% szans, ¿e komisja siê spóŸni
	{
		question_delay = rand() % 50 + 1; // losowy czas (1-50 ms)
		usleep(question_delay * 1000);
	}

	cout << "Komisja zadala pytanie po " << question_delay << " ms." << endl;

	// Symulacja odpowiadania studenta
	int answer_delay = rand() % 91 + 10; // losowy czas (10-100 ms)
	usleep(answer_delay * 1000);

	cout << "Student odpowiedzial po " << answer_delay << " ms." << endl;

	if (!student.practic_pass) // jeœli student nie ma jeszcze zdanej praktyki podchodzi do niej
	{
		int random_practic = rand() % 100; // losowanie liczby z przedzia³u 0-99 w celu ustalenia, czy ocena bêdzie pozytywna

		// losowanie oceny dla praktyki
		if (random_practic < 95) // 95% szans na ocenê pozytywn¹
		{
			int grade_i = rand() % 5;
			student.practic_grade = GRADES[grade_i]; // losuje ocenê od 3.0 do 5.0
		}
		else // 5% szans na ocenê negatywn¹
			student.practic_grade = 2.0;

		// przydzielanie zaliczeñ (jeœli 2.0 zwraca false)
		student.practic_pass = (student.practic_grade > 2.0);
	}
	
	if (student.practic_pass) // jeœli student zda³ praktykê podchodzi do teorii
	{
		int random_theoric = rand() % 100;

		// losowanie oceny dla teorii
		if (random_theoric < 95)
		{
			int grade_i = rand() % 5;
			student.theoric_grade = GRADES[grade_i];
		}
		else
			student.theoric_grade = 2.0;

		student.theoric_pass = (student.theoric_grade > 2.0);
	}
	else
	{
		student.theoric_grade = 2.0; // jeœli praktyka nie zdana - teoria automatycznie tak¿e nie zdana
		student.theoric_pass = false;
	}
	
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
