#include "ipc.h"
#include "utils.h"
#include "constants.h"
#include "structures.h"
#include "exam_logic.h"

void handle_signal(int);
int msgid, semid_stu, semid_start, shmid, shmid_pid;
void* shm_ptr;
void* shm_ptr_pid;
pid_t pid[3] = {};
Exam_result* results;

int main()
{
	signal(SIGINT, handle_signal); // obs�uga sygna�u awaryjnego

	// tworzenie kolejek komunikat�w, semafor�w oraz pami�ci wsp�dzielonej
	key_t msg_key = generate_key('A'); // inicjalizacja kolejki komunikat�w ze wszystkimi
	msgid = create_msg(msg_key);

	key_t shm_key = generate_key('B'); // klucz dla pami�ci wsp�dzielonej z komisjami oraz studentami
	shmid = create_shm(shm_key, sizeof(Student) * MAX_STUDENTS);
	shm_ptr = attach_shm(shmid); // do��czenie pami�ci wsp�dzielonej

	key_t sem_key_stu = generate_key('E'); // klucz do semafor�w ze studentem
	semid_stu = create_sem(sem_key_stu, 1); // tworzymy jeden semafor dla sygnalizacji gotowo�ci od studenta

	key_t sem_key_start = generate_key('X'); // semafor przepuszczaj�cy ka�dego ze student�w
	semid_start = create_sem(sem_key_start, 2); // domy�lnie zero - brak przej�cia

	key_t shm_key_pid = generate_key('Y'); // pomocnicza pami�� wsp�dzielona z pidami student�w
	shmid_pid = create_shm(shm_key_pid, sizeof(Student) * MAX_STUDENTS);
	shm_ptr_pid = attach_shm(shmid_pid);

	cout << purple("[") << purple(to_string(getpid())) << purple("] Dziekan uruchamia proces egzaminu.") << endl;

	// uruchomienie proces�w komisji oraz studenta
	const char* process[] = { "./commissionB", "./commissionA", "./student" };

	for (int i = 0; i < 3; i++)
	{
		pid[i] = fork();
		switch (pid[i])
		{
		case -1:
			handle_error(red("Blad podczas tworzenia procesu"));
		case 0:
			execlp(process[i], process[i], NULL);
			handle_error(red("Blad podczas uruchamiania procesu"));
		default:
			break;
		}
	}

	send_msg(msgid, 40, "START");
	
	string msg = receive_msg(msgid, 50); // informacja o liczbie student�w
	int count = stoi(msg);
	vector<pid_t> child_pids(count); // wektor zawieraj�cy pidy ka�dego ze student�w-proces�w
	memcpy(child_pids.data(), shm_ptr_pid, child_pids.size() * sizeof(pid_t));

	// dziekan wybiera kierunek, kt�ry b�dzie pisa� egzamin
	sem_wait(semid_stu, 0); // czekamy a� studenci b�d� gotowi
	
	int direction = get_direction();

	cout << purple("[") << purple(to_string(getpid())) << purple("] Dziekan wybral kierunek, ktory podejdzie do egzaminu: ") << direction << endl;

	// wys�anie informacji do student�w
	cout << purple("Informacja o kierunku wyslana do studentow.") << endl;
	cout << purple("Oczekiwanie na wyniki od komisji...") << endl << endl;

	send_msg(msgid, 50, to_string(direction));

	srand(static_cast<unsigned int>(time(NULL)));
	int is_alarm = rand() % 2; // 50% szans na wywo�anie alarmu przez dziekana
	if (is_alarm == 1)
		send_signal(child_pids, pid[2], pid[1], pid[0]); // wys�anie sygna�u o alarmie, kt�ry pojawi si� w losowym okresie czasu

	Student* students = static_cast<Student*>(shm_ptr);
	int total_students = 0; // liczba student�w, kt�rzy zako�czyli egzamin
	int student_idx[MAX_STUDENTS] = {}; // tablica przechowuj�ca indeksy student�w z pami�ci

	while (true)
	{
		string result_msg = receive_msg(msgid, 35); // zbieranie komunikat�w od Komisji B
		if (result_msg == "END") break; // ko�czymy p�tl� po otrzymaniu END

		if (result_msg.empty() || !all_of(result_msg.begin(), result_msg.end(), ::isdigit))
			handle_error(red("Nieprawidlowy komunikat od Komisji B"));

		int id = stoi(result_msg);
		bool found = false;

		// szukanie studenta w pami�ci
		for (int i = 0; i < MAX_STUDENTS; ++i)
		{
			if (students[i].id == id)
			{
				student_idx[total_students] = i;
				total_students++;
				found = true;
				break;
			}
		}
		if (!found) handle_error(red("Nie znaleziono studenta o ID: ") + result_msg);
	}

	// przygotowanie wynik�w dla otrzymanych student�w
	Exam_result* results = new Exam_result[total_students];

	for (int i = 0; i < total_students; ++i)
	{
		Student& student = students[student_idx[i]];
		results[i].student_id = student.id;
		results[i].final_grade = calculate_final_grade(student);
		results[i].passed = student.practic_pass && student.theoric_pass;
	}

	// sortowanie wynik�w rosn�co wed�ug ID
	sort(results, results + total_students, [](const Exam_result& a, const Exam_result& b)
		{ return a.student_id < b.student_id; });
		
	// wy�wietlanie wynik�w
	cout << purple("[") << purple(to_string(getpid())) << purple("] Dziekan otrzymal wyniki.") << endl << endl;
	cout << purple("[") << purple(to_string(getpid())) << purple("] Wyniki egzaminu:") << endl;

	int passed_count = 0;
	for (int i = 0; i < total_students; ++i)
	{
		Exam_result& x = results[i];
		cout << purple("Student ID: ") << x.student_id << purple(", Ocena: ") << x.final_grade
			<< purple(", Zaliczenie: ") << (x.passed ? "Tak" : "Nie") << endl;
		if (x.passed) passed_count++;
	}

	// statystyki
	cout << endl << purple("Liczba studentow bioracych udzial w egzaminie: ") << total_students << endl << purple("Ilosc zaliczen: ") 
		<< passed_count << purple(" (") << (total_students > 0 ? 100.0 * passed_count / total_students : 0) << purple("%)") << endl;

	semctl(semid_start, 1, SETVAL, count); // wypuszcamy student�w

	destroy_msg(msgid); // usuni�cie kolejki komunikat�w ze wszystkimi
	destroy_sem(semid_stu); // usuni�cie semafora ze studentem
	destroy_sem(semid_start); // usuni�cie semafora kontroluj�cego student�w-procesy
	detach_shm(shm_ptr); // od�aczenie wska�nika pami�ci wsp�dzielonej ze wszystkimi
	detach_shm(shm_ptr_pid); // od��czenie wska�nika pami�ci z pidami
	destroy_shm(shmid); // usuni�cie pami�ci wsp�dzielonej ze wszystkimi
	destroy_shm(shmid_pid); // usuni�cie pami�ci wsp�dzielonej z pidami
	if (results)
		delete[] results;

	for (int i = 0; i < 3; i++)
	{
		if (waitpid(pid[i], NULL, 0) == -1) // oczekiwanie na zako�czenie wszystkich proces�w potomnych
			handle_error(red("Blad podczas oczekiwania na proces"));
	}

	cout << endl << purple("[") << purple(to_string(getpid())) << purple("] Dziekan konczy proces egzaminu.") << endl << endl;
	
	return 0;
}

void handle_signal(int signum) // obs�uga czyszczenia zasob�w dla dziekana
{
	if (signum == SIGINT)
	{
		for (int i = 0; i < 3; i++)
		{
			if (waitpid(pid[i], NULL, 0) == -1) // oczekiwanie na zako�czenie wszystkich proces�w potomnych
				handle_error(red("Blad podczas oczekiwania na proces"));
		}

		cout << purple("Dziekan przerywa proces egzaminu.") << endl;

		cleanup(msgid, semid_stu, semid_start, shm_ptr, shm_ptr_pid, shmid, shmid_pid); // uniwersalna funkcja czyszcz�ca elementy ipc
		if (results)
			delete[] results;

		cout << purple("Wyczyszczono zasoby dla dziekana.") << endl;
		exit(EXIT_SUCCESS);
	}
}