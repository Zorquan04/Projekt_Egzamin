#include "exam_logic.h"
#include "structures.h"
#include "constants.h"
#include "ipc.h"

vector<pid_t> generate_students(int num_directions, int min_students, int max_students, Student* students)
{
	srand(static_cast<unsigned int>(time(NULL))); // inicjalizacja generatora losowego

	int total_students = 0; // globalny licznik studentów (oraz indeks)
	int student_id = 1; // globalny licznik dla id studentów (dla przydzielania unikalnego ID)
	vector<pid_t> child_pids; // lista PIDów procesów studenckich
	pid_t pid = 0; // zmienna przechowuj¹ca pidy nowych procesów-studentów

	// generowanie wszystkich studentów i dodanie ich do pamiêci wspó³dzielonej - listy
	for (int direction = 0; direction < num_directions; ++direction)
	{
		int num_students = rand() % (max_students - min_students + 1) + min_students; // generowanie liczby studentów odpowiadaj¹cej zakresowi podanemu w argumentach
		for (int i = 0; i < num_students; ++i)
		{
			Student student;
			student.id = student_id++; // przydzielenie unikalnego ID studenta
			student.direction = direction + 1; // przypisanie kierunku studiów

			int random_pass = rand() % 100; // losowanie liczby z przedzia³u 0-99 w celu ustalenia, czy student powtarza egzamin i ma ju¿ zaliczon¹ praktykê
			if (random_pass >= 95) // 5% szans, ¿e student powtarza egzamin i ma ju¿ zdan¹ praktykê
			{
				student.practic_grade = 5.0;
				student.practic_pass = true;
			}
			// dodanie studenta do tablicy - listy
			students[total_students] = student;	

			// stworzenie procesu studenta
			pid = fork();

			if (pid == -1)
			{
				handle_error(red("Blad podczas generowania procesu"));
			}
			else if (pid == 0)
			{
				// utworzenie procesu oraz przes³anie id odpowiedniego studenta jako argument (proces potomny)
				string id = to_string(student.id);
				execlp("./studentExec", "./studentExec", id.c_str(), NULL);
				handle_error(red("Blad podczas uruchamiania procesu"));
			}
			else
			{
				child_pids.push_back(pid); // zapisujemy PID do wektora-tablicy (proces macierzysty)
			}
			total_students++;
		}
	}

	// symulacja losowego pojawiania siê studentów przed uczelni¹
	int remaining = total_students;
	int random[total_students];
	for (int i = 0; i < total_students; ++i)
		random[i] = i;  // wype³nienie tablicy do wyœwietlenia losowego

	for (int i = 0; i < remaining; ++i)
	{
		int rand_index = rand() % remaining;  // losowy wybór indeksu z dostêpnych
		int selec_index = random[rand_index];

		// wyœwietlenie danych wybranego studenta
		cout << green("Przybyl student [") << green(to_string(child_pids[selec_index])) << green("] o ID = ") << students[selec_index].id 
			<< green(", Kierunek: ") << students[selec_index].direction << green(", Powtarza egzamin: ") 
			<< (students[selec_index].practic_pass ? green("Tak") : green("Nie")) << endl;

		// usuniêcie wybranego indeksu poprzez nadpisanie go ostatnim elementem - w celu unikniêcia duplikatów
		random[rand_index] = random[remaining - 1];
		remaining--;

		// losowy czas pojawiania siê studentów (1-10 ms)
		int delay = rand() % 10 + 1;
		usleep(delay * 1000);
	}

	return child_pids;
}

void simulate_answers(Student& student, char x, pid_t pid, int semnum)
{
	/*
	if (x == 'A')
		cout << yellow("Aktualny stan miejsc w Komisji A: ") << yellow(to_string(semnum)) << endl;
	if (x == 'B')
		cout << blue("Aktualny stan miejsc w Komisji B: ") << blue(to_string(semnum)) << endl;
	*/

	// symulacja zadawania pytañ przez komisjê X
	int question_delay = 0;

	if (x == 'B' && !student.practic_pass) // jeœli praktyka nie zdana - teoria automatycznie tak¿e nie zdana
	{
		student.theoric_grade = 2.0; 
		cout << blue("Komisja B oblewa studenta [") << blue(to_string(pid)) << blue("] - praktyka nie zdana") << endl;
		return;
	}
	else if (x == 'A' && student.practic_pass) // jeœli student powtarza egzamin - przechodzi odrazu do teorii
	{
		cout << yellow("Komisja A przepuszcza studenta [") << yellow(to_string(pid)) << yellow("] - powtarza egzamin") << endl;
		return;
	}
	else
	{
		for (int i = 0; i < 3; i++) // symulacja opóŸnieñ przy zadawaniu 3 pytañ i odpowiadaniu
		{
			int delay_chance = rand() % 100; // losowanie liczby z przedzia³u 0 - 99 w celu ustalenia, czy komisja siê spóŸni³a z pytaniem
			int x_question_delay = 0;

			if (delay_chance > 49) // 50% szans, ¿e komisja siê spóŸni
			{
				x_question_delay = rand() % 50 + 1; // losowy czas (1-50 ms)
				usleep(x_question_delay * 1000);
			}
			usleep(ANSWER_TIME * 1000); // 50ms czasu na odpowiedŸ dla studenta

			question_delay += x_question_delay;
		}

		if (x == 'A')
		{
			cout << yellow("Komisja A zadala 3 pytania do [") << yellow(to_string(pid)) << yellow("] po ") << question_delay << yellow(" ms.") << endl
				<< green("Student [") << green(to_string(pid)) << green("] odpowiedzial na 3 pytania praktyczne w wyznaczonym czasie (") << (ANSWER_TIME * 3) << green(" ms).") << endl;
		}
			
		if (x == 'B')
		{
			cout << blue("Komisja B zadala 3 pytania do [") << blue(to_string(pid)) << blue("] po ") << question_delay << blue(" ms.") << endl
				<< green("Student [") << green(to_string(pid)) << green("] odpowiedzial na 3 pytania teoretyczne w wyznaczonym czasie (") << (ANSWER_TIME * 3) << green(" ms).") << endl;
		}	
	}

	if (x == 'A') // wystawienie oceny za praktykê przez przewodnicz¹cego Komisji A
	{
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
	}
	
	if (x == 'B') // wystawienie oceny za teoriê przez przewodnicz¹cego Komisji B
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
