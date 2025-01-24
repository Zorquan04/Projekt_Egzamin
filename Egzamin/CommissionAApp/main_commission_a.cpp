#include "ipc.h"
#include "utils.h"
#include "constants.h"
#include "structures.h"
#include "exam_logic.h"

void handle_signal(int);
int msgid_stu, msgid_com, semid_stu, shmid;
void* shm_ptr;

int main()
{
	signal(SIGINT, handle_signal); // obs³uga sygna³u awaryjnego

	// tworzenie kolejek komunikatów, semaforów oraz pamiêci wspó³dzielonej
	key_t msg_key_stu = generate_key('B'); // inicjalizacja kolejki komunikatów ze studentami
	msgid_stu = create_msg(msg_key_stu);

	key_t msg_key_com = generate_key('C'); // inicjalizacja kolejki komunikatów z komisj¹ B
	msgid_com = create_msg(msg_key_com);

	key_t sem_key_stu = generate_key('F'); // klucz do semaforów
	semid_stu = create_sem(sem_key_stu, 2); // zestaw semaforów: 0 - komisja A, 1 - studenci
	semctl(semid_stu, 0, SETVAL, 1); // ustawienie wartoœci domyœlnej dla semafora odpowiedzialnego za komisjê A

	key_t sem_key_com = generate_key('G'); // klucz do semaforów
	int semid_com = create_sem(sem_key_com, 2); // zestaw semaforów: 0 - komisja A, 1 - komisja B

	key_t shm_key = generate_key('H'); // inicjalizacja pamiêci wspó³dzielonej ze studentem, komisj¹ B oraz dziekanem
	shmid = create_shm(shm_key, sizeof(Student) * MAX_STUDENTS); // tworzenie pamiêci wspó³dzielonej
	shm_ptr = attach_shm(shmid); // do³¹czanie pamiêci wspó³dzielonej

	usleep(1500000);

	cout << "[" << getpid() << "] Komisja A gotowa do pracy." << endl;

	// odbieranie studentów z pamiêci wspó³dzielonej
	Student* students = static_cast<Student*>(shm_ptr);

	// przeprowadzanie czêœci praktycznej
	while (true)
	{
		usleep(500000); // argument w mikrosekundach - 0.5s

		sem_wait(semid_stu, 1); // oczekiwanie na powiadomienie od studentów

		for (int i = 0; i < 3; ++i) // egzaminowanie trójki studentów
		{
			string student_msg = receive_msg(msgid_stu, 1); // odbieranie komunikatu o studentach
			if (student_msg == "END") // sprawdzenie czy ju¿ komunikat koñcowy
			{
				cout << "[" << getpid() << "] Wyslanie studentow do komisji B." << endl;
				send_msg(msgid_com, 1, "END"); // powiadomienie komisji B o zakoñczeniu pracy ze studentami
				sem_signal(semid_com, 1);
				goto end_exam;
			}

			int index = 0; // zmienna przechowuj¹ca id studenta otrzymanego z komunikatu
			if (!student_msg.empty() && all_of(student_msg.begin(), student_msg.end(), ::isdigit)) // sprawdzenie poprawnoœci danych komunikatu
				index = stoi(student_msg); // student_msg zawiera index studenta w pamiêci wspó³dzielonej
			else
				handle_error("Nieprawidlowy komunikat");

			Student student = students[index]; // przypisanie studenta do odpowiadaj¹cego studenta z pamiêci wspó³dzielonej - listy

			cout << "Komisja A przyjmuje studenta o ID = " << student.id << endl;

			simulate_answers(student, 'A'); // symulujemy zadawanie pytañ i odpowiedzi dla studenta z listy

			// zapisanie wyników do pamiêci
			students[index].practic_grade = student.practic_grade;
			students[index].practic_pass = student.practic_pass;

			cout << "Komisja A ocenila studenta o ID = " << student.id << " za praktyke: " << student.practic_grade << endl << endl;

			string result_msg = to_string(index);
			send_msg(msgid_com, 1, result_msg); // wysy³anie do komisji B
		}

		cout << "[" << getpid() << "] Wyslanie studentow do komisji B." << endl;
		
		sem_signal(semid_com, 1); // powiadomienie komisji B, ¿e max trójka studentów jest gotowa
		sem_wait(semid_com, 0); // oczekiwanie na zakoñczenie egzaminowania przez komisjê B

		sem_signal(semid_stu, 0); // poinformowanie studentów o wolnej komisji
	}

	end_exam:
	cout << "[" << getpid() << "] Komisja A przeslala wszystkie wyniki do Komisji B." << endl;

	destroy_msg(msgid_stu); // usuniêcie kolejki komunikatów ze studentami
	destroy_sem(semid_stu); // usuniêcie semaforów do obs³ugi studentów
	detach_shm(shm_ptr); // od³aczenie wskaŸnika pamiêci wspó³dzielonej ze studentem, komisj¹ B oraz dziekanem

	// cleanup(msgid_stu, semid_stu, shm_ptr, shmid_stu); lub z pomoc¹ funkcji cleanup

	cout << "[" << getpid() << "] Komisja A konczy prace." << endl << endl;

	return 0;
}

void handle_signal(int signum)  // obs³uga czyszczenia zasobów dla komisji A
{
	usleep(500000);
	send_msg(msgid_com, 1, "END"); // awaryjne powiadomienie komisji B o zakoñczeniu egzaminowania
	cout << "Komisja A przerywa proces egzaminu." << endl;
	cleanup(msgid_stu, semid_stu, shm_ptr, -1); // uniwersalna funkcja czyszcz¹ca elementy ipc
	cout << "Wyczyszczono zasoby dla komisji A." << endl;
	exit(EXIT_SUCCESS);
}
