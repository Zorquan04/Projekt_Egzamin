#include "ipc.h"
#include "utils.h"
#include "constants.h"
#include "structures.h"
#include "exam_logic.h"

void handle_signal(int);
int msgid_com, semid_stu, shmid;
void* shm_ptr;

int main()
{
	signal(SIGINT, handle_signal); // obs�uga sygna�u awaryjnego

	// tworzenie kolejek komunikat�w, semafor�w oraz pami�ci wsp�dzielonej
	key_t key_stu = generate_key('B'); // inicjalizacja kolejki komunikat�w ze studentami
	int msgid_stu = create_msg(key_stu);

	key_t key_com = generate_key('C'); // inicjalizacja kolejki komunikat�w z komisj�
	msgid_com = create_msg(key_com);

	key_t sem_key_stu = generate_key('E'); // klucz do semafor�w ze studentem
	semid_stu = create_sem(sem_key_stu, 1); // tworzymy jeden semafor dla sygnalizacji gotowo�ci od studenta

	key_t shm_key = generate_key('G'); // klucz dla pami�ci wsp�dzielonej z wynikami od komisji
	shmid = create_shm(shm_key, sizeof(Exam_result) * MAX_STUDENTS);
	shm_ptr = attach_shm(shmid); // do��czenie pami�ci wsp�dzielonej

	cout << "[" << getpid() << "] Dziekan uruchamia proces egzaminu." << endl;

	// uruchomienie proces�w komisji oraz studenta
	const char* process[] = {"./commission", "./student" };
	pid_t pid[2] = {};

	for (int i = 0; i < 2; i++)
	{
		pid[i] = fork();
		switch (pid[i])
		{
		case -1:
			handle_error("Blad podczas tworzenia procesu");
		case 0:
			execlp(process[i], process[i], NULL);
			handle_error("Blad podczas uruchamiania procesu");
		default:
			break;
		}
	}

	// dziekan wybiera kierunek, kt�ry b�dzie pisa� egzamin
	sem_wait(semid_stu, 0); // czekamy a� studenci b�d� gotowi
	sleep(1);
	int direction = get_direction(); // u�ytkownik wybiera kierunek

	cout << "[" << getpid() << "] Dziekan wybral kierunek, ktory podejdzie do egzaminu: " << direction << endl;

	// wys�anie informacji do student�w
	send_msg(msgid_stu, 3, to_string(direction));
	cout << "Informacja o kierunku wyslana do studentow." << endl;

	cout << "Oczekiwanie na wyniki od komisji..." << endl << endl;
	
	srand(static_cast<unsigned int>(time(NULL)));
	int is_alarm = rand() % 2; // 50% szans na wywo�anie alarmu przez dziekana
	if (is_alarm == 1)
		send_signal(pid[1], pid[0], getpid()); // wys�anie sygna�u o alarmie, kt�ry pojawi si� w losowym okresie czasu

	// odczytanie wynik�w z pami�ci wsp�dzielonej
	Exam_result* results = static_cast<Exam_result*>(shm_ptr); // przy��czenie si� do pami�ci z wynikami
	int index = 0;

	while (true)
	{
		string result_msg = receive_msg(msgid_com, 2); // otrzymanie danych od komisji
		if (result_msg == "END") break; // pobieraj dane a� do odnaleziena END symbolizuj�cego koniec danych

		++index; // liczenie ��cznej liczby student�w
	}

	usleep(250000);
	cout << "[" << getpid() << "] Dziekan otrzymal wyniki od komisji." << endl << endl;
	sleep(2);

	// wypisanie wynik�w egzaminu
	cout << "[" << getpid() << "] Publikowanie wynikow:" << endl;

	int total_students = index;  // ��czna liczba student�w
	int passed_students = 0; // liczba zalicze�

	for (int i = 0; i < total_students; ++i)
	{
		Exam_result result = results[i]; // wypisywanie wynik�w pozyskanych z pami�ci komisji
		cout << "Student ID: " << result.student_id << ", Ocena: " << result.final_grade
			<< ", Zaliczenie: " << (result.passed ? "Tak" : "Nie") << endl;

		if (result.passed)
			passed_students++; // zliczanie zalicze�

		usleep(100000);
	}

	sleep(1);
	cout << endl << "Liczba studentow, ktorzy podeszli do egzaminu: " << total_students << endl;
	cout << "Liczba studentow, ktorzy zdali: " << passed_students << " (" << (100.0 * passed_students / total_students) << "%)" << endl;

	sleep(1);
	destroy_msg(msgid_com); // usuni�cie kolejki komunikat�w z komisj�
	destroy_sem(semid_stu); // usuni�cie semafora ze studentami
	detach_shm(shm_ptr); // od�aczenie wska�nika pami�ci wsp�dzielonej z komisj�
	destroy_shm(shmid); // usuni�cie pami�ci wsp�dzielonej z komisj�

	for (int i = 0; i < 2; i++)
	{
		if (waitpid(pid[i], NULL, 0) == -1) // oczekiwanie na zako�czenie wszystkich proces�w potomnych
			handle_error("Blad podczas oczekiwania na proces");	
	}

	// cleanup(msgid_com, semid_stu, shm_ptr, NULL, shmid); lub z pomoc� funkcji cleanup

	cout << endl << "[" << getpid() << "] Dziekan konczy proces egzaminu." << endl << endl;
	
	return 0;
}

void handle_signal(int signum) // obs�uga czyszczenia zasob�w dla dziekana
{
	cout << "Dziekan przerywa proces egzaminu." << endl;
	cleanup(msgid_com, semid_stu, shm_ptr, NULL, shmid); // uniwersalna funkcja czyszcz�ca elementy ipc
	cout << "Wyczyszczono zasoby dla dziekana." << endl;
	exit(EXIT_SUCCESS);
}