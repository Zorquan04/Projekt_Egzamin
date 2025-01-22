#include "ipc.h"
#include "utils.h"
#include "constants.h"
#include "structures.h"
#include "exam_logic.h"

void handle_signal(int);
int msgid_stu, msgid_dean, semid, shmid_stu;
void* shm_ptr_stu;
void* shm_ptr_dean;

int main()
{
	signal(SIGINT, handle_signal); // obs³uga sygna³u awaryjnego

	// tworzenie kolejek komunikatów, semaforów oraz pamiêci wspó³dzielonej
	key_t msg_key_stu = generate_key('A'); // inicjalizacja kolejki komunikatów ze studentami
	msgid_stu = create_msg(msg_key_stu);

	key_t msg_key_dean = generate_key('C'); // inicjalizacja kolejki komunikatów z dziekanem
	msgid_dean = create_msg(msg_key_dean);

	key_t sem_key = generate_key('D'); // klucz do semaforów
	semid = create_sem(sem_key, 2); // zestaw semaforów: 0 - komisja, 1 - studenci
	semctl(semid, 0, SETVAL, 1); // ustawienie wartoœci domyœlnej dla semafora odpowiedzialnego za komisjê

	key_t shm_key_stu = generate_key('F'); // inicjalizacja pamiêci wspó³dzielonej ze studentem
	shmid_stu = create_shm(shm_key_stu, sizeof(Student) * MAX_STUDENTS); // tworzenie pamiêci wspó³dzielonej
	shm_ptr_stu = attach_shm(shmid_stu); // do³¹czanie pamiêci wspó³dzielonej

	key_t shm_key_dean = generate_key('G'); // inicjalizacja pamiêci wspó³dzielonej z dziekanem
	int shmid_dean = create_shm(shm_key_dean, sizeof(Exam_result) * MAX_STUDENTS);
	shm_ptr_dean = attach_shm(shmid_dean);

	sleep(1);

	cout << "[" << getpid() << "] Komisja gotowa do pracy." << endl;

	// odbieranie studentów z pamiêci wspó³dzielonej
	Student* students = static_cast<Student*>(shm_ptr_stu);
	Exam_result* results = static_cast<Exam_result*>(shm_ptr_dean);  // tablica wyników w pamiêci wspó³dzielonej
	int index = 0;

	while (true)
	{
		usleep(250000); // argument w mikrosekundach - 0.25s

		sem_wait(semid, 1); // oczekiwanie na powiadomienie od studentów

		string student_msg = receive_msg(msgid_stu, 1); // odbieranie komunikatu o studencie
		if (student_msg == "END") break; // sprawdzenie czy ju¿ komunikatu koñcowy

		cout << "Komisja przyjmuje studenta o ID = " << student_msg << endl;

		Student student = students[index]; // przypisanie studenta do odpowiadaj¹cego studenta z pamiêci wspó³dzielonej

		if (!student_msg.empty() && all_of(student_msg.begin(), student_msg.end(), ::isdigit)) // sprawdzenie poprawnoœci danych komunikatu
			student.id = stoi(student_msg); // student_msg zawiera id studenta
		else
			handle_error("Nieprawidlowy komunikat studenta");

		simulate_answers(student); // symulujemy zadawanie pytañ i odpowiedzi

		cout << "Komisja ocenila studenta o ID = " << student.id << ":" << endl;
		cout << "Ocena za czesc praktyczna: " << student.practic_grade << endl;
		cout << "Ocena za czesc teoretyczna: " << student.theoric_grade << endl << endl;

		// wystawienie oceny za egzamin
		Exam_result result;
		result.student_id = student.id;
		result.final_grade = calculate_final_grade(student);
		result.passed = student.practic_pass && student.theoric_pass;

		results[index] = result;  // zapisanie wyniku w pamiêci wspó³dzielonej

		string result_msg = to_string(result.student_id) + " " + // konwersja na string w celu umieszczenia wyników w komunikacie
			to_string(result.final_grade) + " " + to_string(result.passed);

		send_msg(msgid_dean, 2, result_msg); // wysy³anie wyników do dziekana

		sem_signal(semid, 0); // powiadomienie studentów o dostêpnoœci komisji

		++index; // zwiêkszenie indeksu w celu pracy z kolejnym studentem z pamiêci wspó³dzielonej
	}

	sem_signal(semid, 1); // sygna³ koñcowy
	cout << "[" << getpid() << "] Komisja przeslala wyniki do dziekana." << endl;
	send_msg(msgid_dean, 2, "END"); // powiadomienie dziekana o zakoñczeniu

	destroy_msg(msgid_stu); // usuniêcie kolejki komunikatów ze studentami
	destroy_sem(semid); // usuniêcie semaforów do obs³ugi studentów
	detach_shm(shm_ptr_stu); // od³aczenie wskaŸnika pamiêci wspó³dzielonej ze studentem
	detach_shm(shm_ptr_dean); // od³aczenie wskaŸnika pamiêci wspó³dzielonej z dziekanem
	destroy_shm(shmid_stu); // usuniêcie pamiêci wspó³dzielonej ze studentem

	// cleanup(msgid_stu, semid, shm_ptr_stu, shm_ptr_dean, shmid_stu); lub z pomoc¹ funkcji cleanup

	cout << "[" << getpid() << "] Komisja konczy prace." << endl << endl;

	return 0;
}

void handle_signal(int signum)  // obs³uga czyszczenia zasobów dla komisji
{
	send_msg(msgid_dean, 2, "END"); // awaryjne powiadomienie dziekana o zakoñczeniu egzaminowania
	cout << "Komisja przerywa proces egzaminu." << endl;
	cleanup(msgid_stu, semid, shm_ptr_stu, shm_ptr_dean, shmid_stu); // uniwersalna funkcja czyszcz¹ca elementy ipc
	cout << "Wyczyszczono zasoby dla komisji." << endl;
	exit(EXIT_SUCCESS);
}