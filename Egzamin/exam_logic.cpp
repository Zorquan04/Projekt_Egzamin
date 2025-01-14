#include "exam_logic.h"
#include "structures.h"

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

			// symulacja losowego czasu pojawiania si� student�w
			int delay = rand() % 91 + 10; // losowy czas w milisekundach (10-100 ms)
			usleep(delay * 1000); // konwersja na mikrosekundy
			cout << students.back() << endl; // odwo�anie do ostatnio dodanego indeksu w wektorze w celu jego wy�wietlenia
		}
	}
}

void simulate_answers(Student& student)
{
	// losowanie liczby z przedzia�u 0-99 w celu ustalenia, czy ocena b�dzie pozytywna
	int random_practic = rand() % 100;
	int random_theoric = rand() % 100;

	// losowanie oceny dla praktyki
	if (random_practic < 95) // 95% szans na ocen� pozytywn�
	{
		student.practic_grade = static_cast<float>(3.0 + rand() % 5 * 0.5); // losuje ocen� od 3.0 do 5.0
	}
	else // 5% szans na ocen� negatywn�
	{
		student.practic_grade = 2.0;
	}

	// losowanie oceny dla teorii
	if (random_theoric < 95)
	{
		student.theoric_grade = static_cast<float>(3.0 + rand() % 5 * 0.5);
	}
	else
	{
		student.theoric_grade = 2.0;
	}

	// przydzielanie zalicze� (je�li 2.0 zwraca false)
	student.practic_pass = (student.practic_grade > 2.0);
	student.theoric_pass = (student.theoric_grade > 2.0);
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
