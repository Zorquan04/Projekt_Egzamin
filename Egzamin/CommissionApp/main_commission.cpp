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
	signal(SIGINT, handle_signal); // obs�uga sygna�u awaryjnego

	// tworzenie kolejek komunikat�w, semafor�w oraz pami�ci wsp�dzielonej
	key_t msg_key_stu = generate_key('A'); // inicjalizacja kolejki komunikat�w ze studentami
	msgid_stu = create_msg(msg_key_stu);

	key_t msg_key_dean = generate_key('C'); // inicjalizacja kolejki komunikat�w z dziekanem
	msgid_dean = create_msg(msg_key_dean);

	key_t sem_key = generate_key('D'); // klucz do semafor�w
	semid = create_sem(sem_key, 2); // zestaw semafor�w: 0 - komisja, 1 - studenci
	semctl(semid, 0, SETVAL, 1); // ustawienie warto�ci domy�lnej dla semafora odpowiedzialnego za komisj�

	key_t shm_key_stu = generate_key('F'); // inicjalizacja pami�ci wsp�dzielonej ze studentem
	shmid_stu = create_shm(shm_key_stu, sizeof(Student) * MAX_STUDENTS); // tworzenie pami�ci wsp�dzielonej
	shm_ptr_stu = attach_shm(shmid_stu); // do��czanie pami�ci wsp�dzielonej

	key_t shm_key_dean = generate_key('G'); // inicjalizacja pami�ci wsp�dzielonej z dziekanem
	int shmid_dean = create_shm(shm_key_dean, sizeof(Exam_result) * MAX_STUDENTS);
	shm_ptr_dean = attach_shm(shmid_dean);

	sleep(1);

	cout << "[" << getpid() << "] Komisja gotowa do pracy." << endl;

	// odbieranie student�w z pami�ci wsp�dzielonej
	Student* students = static_cast<Student*>(shm_ptr_stu);
	Exam_result* results = static_cast<Exam_result*>(shm_ptr_dean);  // tablica wynik�w w pami�ci wsp�dzielonej
	int index = 0;

	while (true)
	{
		usleep(250000); // argument w mikrosekundach - 0.25s

		sem_wait(semid, 1); // oczekiwanie na powiadomienie od student�w

		string student_msg = receive_msg(msgid_stu, 1); // odbieranie komunikatu o studencie
		if (student_msg == "END") break; // sprawdzenie czy ju� komunikatu ko�cowy

		cout << "Komisja przyjmuje studenta o ID = " << student_msg << endl;

		Student student = students[index]; // przypisanie studenta do odpowiadaj�cego studenta z pami�ci wsp�dzielonej

		if (!student_msg.empty() && all_of(student_msg.begin(), student_msg.end(), ::isdigit)) // sprawdzenie poprawno�ci danych komunikatu
			student.id = stoi(student_msg); // student_msg zawiera id studenta
		else
			handle_error("Nieprawidlowy komunikat studenta");

		simulate_answers(student); // symulujemy zadawanie pyta� i odpowiedzi

		cout << "Komisja ocenila studenta o ID = " << student.id << ":" << endl;
		cout << "Ocena za czesc praktyczna: " << student.practic_grade << endl;
		cout << "Ocena za czesc teoretyczna: " << student.theoric_grade << endl << endl;

		// wystawienie oceny za egzamin
		Exam_result result;
		result.student_id = student.id;
		result.final_grade = calculate_final_grade(student);
		result.passed = student.practic_pass && student.theoric_pass;

		results[index] = result;  // zapisanie wyniku w pami�ci wsp�dzielonej

		string result_msg = to_string(result.student_id) + " " + // konwersja na string w celu umieszczenia wynik�w w komunikacie
			to_string(result.final_grade) + " " + to_string(result.passed);

		send_msg(msgid_dean, 2, result_msg); // wysy�anie wynik�w do dziekana

		sem_signal(semid, 0); // powiadomienie student�w o dost�pno�ci komisji

		++index; // zwi�kszenie indeksu w celu pracy z kolejnym studentem z pami�ci wsp�dzielonej
	}

	sem_signal(semid, 1); // sygna� ko�cowy
	cout << "[" << getpid() << "] Komisja przeslala wyniki do dziekana." << endl;
	send_msg(msgid_dean, 2, "END"); // powiadomienie dziekana o zako�czeniu

	destroy_msg(msgid_stu); // usuni�cie kolejki komunikat�w ze studentami
	destroy_sem(semid); // usuni�cie semafor�w do obs�ugi student�w
	detach_shm(shm_ptr_stu); // od�aczenie wska�nika pami�ci wsp�dzielonej ze studentem
	detach_shm(shm_ptr_dean); // od�aczenie wska�nika pami�ci wsp�dzielonej z dziekanem
	destroy_shm(shmid_stu); // usuni�cie pami�ci wsp�dzielonej ze studentem

	// cleanup(msgid_stu, semid, shm_ptr_stu, shm_ptr_dean, shmid_stu); lub z pomoc� funkcji cleanup

	cout << "[" << getpid() << "] Komisja konczy prace." << endl << endl;

	return 0;
}

void handle_signal(int signum)  // obs�uga czyszczenia zasob�w dla komisji
{
	send_msg(msgid_dean, 2, "END"); // awaryjne powiadomienie dziekana o zako�czeniu egzaminowania
	cout << "Komisja przerywa proces egzaminu." << endl;
	cleanup(msgid_stu, semid, shm_ptr_stu, shm_ptr_dean, shmid_stu); // uniwersalna funkcja czyszcz�ca elementy ipc
	cout << "Wyczyszczono zasoby dla komisji." << endl;
	exit(EXIT_SUCCESS);
}