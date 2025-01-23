#include "ipc.h"
#include "utils.h"
#include "constants.h"
#include "structures.h"
#include "exam_logic.h"

void handle_signal(int);
int msgid_com, semid, shmid;
void* shm_ptr;
Exam_result* results;

int main()
{
	signal(SIGINT, handle_signal); // obs³uga sygna³u awaryjnego

	// tworzenie kolejek komunikatów, semaforów oraz pamiêci wspó³dzielonej
	key_t msg_key_stu = generate_key('A'); // inicjalizacja kolejki komunikatów ze studentami
	int msgid_stu = create_msg(msg_key_stu);

	key_t msg_key_com = generate_key('D'); // inicjalizacja kolejki komunikatów z komisj¹ B
	msgid_com = create_msg(msg_key_com);

	key_t sem_key = generate_key('E'); // klucz do semaforów ze studentem
	semid = create_sem(sem_key, 1); // tworzymy jeden semafor dla sygnalizacji gotowoœci od studenta

	key_t shm_key = generate_key('H'); // klucz dla pamiêci wspó³dzielonej z komisj¹ oraz studentami
	shmid = create_shm(shm_key, sizeof(Student) * MAX_STUDENTS);
	shm_ptr = attach_shm(shmid); // do³¹czenie pamiêci wspó³dzielonej

	cout << "[" << getpid() << "] Dziekan uruchamia proces egzaminu." << endl;

	// uruchomienie procesów komisji oraz studenta
	const char* process[] = { "./commissionB", "./commissionA", "./student" };
	pid_t pid[3] = {};

	for (int i = 0; i < 3; i++)
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
	sem_wait(semid, 0); // czekamy a¿ studenci bêd¹ gotowi
	sleep(1);
	int direction = get_direction(); // u¿ytkownik wybiera kierunek

	cout << "[" << getpid() << "] Dziekan wybral kierunek, ktory podejdzie do egzaminu: " << direction << endl;

	// wys³anie informacji do studentów
	send_msg(msgid_stu, 1, to_string(direction));
	cout << "Informacja o kierunku wyslana do studentow." << endl;

	cout << "Oczekiwanie na wyniki od komisji..." << endl << endl;
	
	srand(static_cast<unsigned int>(time(NULL)));
	int is_alarm = rand() % 2; // 50% szans na wywo³anie alarmu przez dziekana
	if (is_alarm == 1)
		send_signal(pid[2], pid[1], pid[0]); // wys³anie sygna³u o alarmie, który pojawi siê w losowym okresie czasu

	Student* students = static_cast<Student*>(shm_ptr); // przy³¹czenie siê do pamiêci z wynikami
	int total_students = 0; // ³¹czna liczba studentów

	while (true) // czekanie na zakoñczenie egzaminu
	{
		string result_msg = receive_msg(msgid_com, 1); // otrzymywanie informacji o studentach koñcz¹cych egzamin
		if (result_msg == "END") break; // czekanie na komunikat koñca od komisji B

		if (!result_msg.empty() && all_of(result_msg.begin(), result_msg.end(), ::isdigit)) // sprawdzenie poprawnoœci danych komunikatu
			total_students = stoi(result_msg); // student_msg zawiera id studenta
		else
			handle_error("Nieprawidlowy komunikat");
	}

	// przyporz¹dkowywanie oceny finalnej oraz zaliczenia ka¿demu studentowi z pamiêci wspóldzielonej
	results = new Exam_result[MAX_STUDENTS * 5]; // struktura listy z wynikami

	for (int i = 0; i < total_students; ++i)
	{
		results[i].student_id = students[i].id;
		results[i].final_grade = calculate_final_grade(students[i]);
		results[i].passed = students[i].practic_pass && students[i].theoric_pass;
	}

	usleep(250000);
	cout << "[" << getpid() << "] Dziekan otrzymal wyniki od komisji." << endl << endl;
	sleep(2);

	// wypisanie wyników egzaminu
	cout << "[" << getpid() << "] Publikowanie wynikow:" << endl;
 
	int passed_students = 0; // liczba zaliczeñ

	for (int i = 0; i < total_students; ++i)
	{
		Exam_result result = results[i]; // wypisywanie wyników pozyskanych z pamiêci komisji
		cout << "Student ID: " << result.student_id << ", Ocena: " << result.final_grade
			<< ", Zaliczenie: " << (result.passed ? "Tak" : "Nie") << endl;

		if (result.passed)
			passed_students++; // zliczanie zaliczeñ

		usleep(50000); // 0.05s opóŸnienia
	}

	sleep(1);
	cout << endl << "Liczba studentow, ktorzy podeszli do egzaminu: " << total_students << endl;
	cout << "Liczba studentow, ktorzy zdali: " << passed_students << " (" << (100.0 * passed_students / total_students) << "%)" << endl;

	sleep(1);
	destroy_msg(msgid_com); // usuniêcie kolejki komunikatów z komisj¹
	destroy_sem(semid); // usuniêcie semafora ze studentami
	detach_shm(shm_ptr); // od³aczenie wskaŸnika pamiêci wspó³dzielonej z komisj¹
	destroy_shm(shmid); // usuniêcie pamiêci wspó³dzielonej z komisj¹ i studentami
	delete[] results;

	for (int i = 0; i < 2; i++)
	{
		if (waitpid(pid[i], NULL, 0) == -1) // oczekiwanie na zakoñczenie wszystkich procesów potomnych
			handle_error("Blad podczas oczekiwania na proces");	
	}

	// cleanup(msgid_com, semid, shm_ptr, shmid); lub z pomoc¹ funkcji cleanup

	cout << endl << "[" << getpid() << "] Dziekan konczy proces egzaminu." << endl << endl;
	
	return 0;
}

void handle_signal(int signum) // obs³uga czyszczenia zasobów dla dziekana
{
	sleep(1);
	cout << "Dziekan przerywa proces egzaminu." << endl;
	cleanup(msgid_com, semid, shm_ptr, shmid); // uniwersalna funkcja czyszcz¹ca elementy ipc
	if (results)
		delete[] results;
	cout << "Wyczyszczono zasoby dla dziekana." << endl;
	exit(EXIT_SUCCESS);
}