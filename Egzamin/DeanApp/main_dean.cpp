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
	signal(SIGINT, handle_signal); // obs³uga sygna³u awaryjnego

	// tworzenie kolejek komunikatów, semaforów oraz pamiêci wspó³dzielonej
	key_t key_stu = generate_key('B'); // inicjalizacja kolejki komunikatów ze studentami
	int msgid_stu = create_msg(key_stu);

	key_t key_com = generate_key('C'); // inicjalizacja kolejki komunikatów z komisj¹
	msgid_com = create_msg(key_com);

	key_t sem_key_stu = generate_key('E'); // klucz do semaforów ze studentem
	semid_stu = create_sem(sem_key_stu, 1); // tworzymy jeden semafor dla sygnalizacji gotowoœci od studenta

	key_t shm_key = generate_key('G'); // klucz dla pamiêci wspó³dzielonej z wynikami od komisji
	shmid = create_shm(shm_key, sizeof(Exam_result) * MAX_STUDENTS);
	shm_ptr = attach_shm(shmid); // do³¹czenie pamiêci wspó³dzielonej

	cout << "[" << getpid() << "] Dziekan uruchamia proces egzaminu." << endl;

	// uruchomienie procesów komisji oraz studenta
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

	// dziekan wybiera kierunek, który bêdzie pisa³ egzamin
	sem_wait(semid_stu, 0); // czekamy a¿ studenci bêd¹ gotowi
	sleep(1);
	int direction = get_direction(); // u¿ytkownik wybiera kierunek

	cout << "[" << getpid() << "] Dziekan wybral kierunek, ktory podejdzie do egzaminu: " << direction << endl;

	// wys³anie informacji do studentów
	send_msg(msgid_stu, 3, to_string(direction));
	cout << "Informacja o kierunku wyslana do studentow." << endl;

	cout << "Oczekiwanie na wyniki od komisji..." << endl << endl;
	
	srand(static_cast<unsigned int>(time(NULL)));
	int is_alarm = rand() % 2; // 50% szans na wywo³anie alarmu przez dziekana
	if (is_alarm == 1)
		send_signal(pid[1], pid[0], getpid()); // wys³anie sygna³u o alarmie, który pojawi siê w losowym okresie czasu

	// odczytanie wyników z pamiêci wspó³dzielonej
	Exam_result* results = static_cast<Exam_result*>(shm_ptr); // przy³¹czenie siê do pamiêci z wynikami
	int index = 0;

	while (true)
	{
		string result_msg = receive_msg(msgid_com, 2); // otrzymanie danych od komisji
		if (result_msg == "END") break; // pobieraj dane a¿ do odnaleziena END symbolizuj¹cego koniec danych

		++index; // liczenie ³¹cznej liczby studentów
	}

	usleep(250000);
	cout << "[" << getpid() << "] Dziekan otrzymal wyniki od komisji." << endl << endl;
	sleep(2);

	// wypisanie wyników egzaminu
	cout << "[" << getpid() << "] Publikowanie wynikow:" << endl;

	int total_students = index;  // ³¹czna liczba studentów
	int passed_students = 0; // liczba zaliczeñ

	for (int i = 0; i < total_students; ++i)
	{
		Exam_result result = results[i]; // wypisywanie wyników pozyskanych z pamiêci komisji
		cout << "Student ID: " << result.student_id << ", Ocena: " << result.final_grade
			<< ", Zaliczenie: " << (result.passed ? "Tak" : "Nie") << endl;

		if (result.passed)
			passed_students++; // zliczanie zaliczeñ

		usleep(100000);
	}

	sleep(1);
	cout << endl << "Liczba studentow, ktorzy podeszli do egzaminu: " << total_students << endl;
	cout << "Liczba studentow, ktorzy zdali: " << passed_students << " (" << (100.0 * passed_students / total_students) << "%)" << endl;

	sleep(1);
	destroy_msg(msgid_com); // usuniêcie kolejki komunikatów z komisj¹
	destroy_sem(semid_stu); // usuniêcie semafora ze studentami
	detach_shm(shm_ptr); // od³aczenie wskaŸnika pamiêci wspó³dzielonej z komisj¹
	destroy_shm(shmid); // usuniêcie pamiêci wspó³dzielonej z komisj¹

	for (int i = 0; i < 2; i++)
	{
		if (waitpid(pid[i], NULL, 0) == -1) // oczekiwanie na zakoñczenie wszystkich procesów potomnych
			handle_error("Blad podczas oczekiwania na proces");	
	}

	// cleanup(msgid_com, semid_stu, shm_ptr, NULL, shmid); lub z pomoc¹ funkcji cleanup

	cout << endl << "[" << getpid() << "] Dziekan konczy proces egzaminu." << endl << endl;
	
	return 0;
}

void handle_signal(int signum) // obs³uga czyszczenia zasobów dla dziekana
{
	cout << "Dziekan przerywa proces egzaminu." << endl;
	cleanup(msgid_com, semid_stu, shm_ptr, NULL, shmid); // uniwersalna funkcja czyszcz¹ca elementy ipc
	cout << "Wyczyszczono zasoby dla dziekana." << endl;
	exit(EXIT_SUCCESS);
}