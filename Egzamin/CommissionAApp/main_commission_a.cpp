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
	signal(SIGINT, handle_signal); // obs�uga sygna�u awaryjnego

	// tworzenie kolejek komunikat�w, semafor�w oraz pami�ci wsp�dzielonej
	key_t msg_key_stu = generate_key('B'); // inicjalizacja kolejki komunikat�w ze studentami
	msgid_stu = create_msg(msg_key_stu);

	key_t msg_key_com = generate_key('C'); // inicjalizacja kolejki komunikat�w z komisj� B
	msgid_com = create_msg(msg_key_com);

	key_t sem_key_stu = generate_key('F'); // klucz do semafor�w
	semid_stu = create_sem(sem_key_stu, 2); // zestaw semafor�w: 0 - komisja A, 1 - studenci
	semctl(semid_stu, 0, SETVAL, 1); // ustawienie warto�ci domy�lnej dla semafora odpowiedzialnego za komisj� A

	key_t sem_key_com = generate_key('G'); // klucz do semafor�w
	int semid_com = create_sem(sem_key_com, 2); // zestaw semafor�w: 0 - komisja A, 1 - komisja B

	key_t shm_key = generate_key('H'); // inicjalizacja pami�ci wsp�dzielonej ze studentem, komisj� B oraz dziekanem
	shmid = create_shm(shm_key, sizeof(Student) * MAX_STUDENTS); // tworzenie pami�ci wsp�dzielonej
	shm_ptr = attach_shm(shmid); // do��czanie pami�ci wsp�dzielonej

	usleep(1500000);

	cout << "[" << getpid() << "] Komisja A gotowa do pracy." << endl;

	// odbieranie student�w z pami�ci wsp�dzielonej
	Student* students = static_cast<Student*>(shm_ptr);

	// przeprowadzanie cz�ci praktycznej
	while (true)
	{
		usleep(500000); // argument w mikrosekundach - 0.5s

		sem_wait(semid_stu, 1); // oczekiwanie na powiadomienie od student�w

		for (int i = 0; i < 3; ++i) // egzaminowanie tr�jki student�w
		{
			string student_msg = receive_msg(msgid_stu, 1); // odbieranie komunikatu o studentach
			if (student_msg == "END") // sprawdzenie czy ju� komunikat ko�cowy
			{
				cout << "[" << getpid() << "] Wyslanie studentow do komisji B." << endl;
				send_msg(msgid_com, 1, "END"); // powiadomienie komisji B o zako�czeniu pracy ze studentami
				sem_signal(semid_com, 1);
				goto end_exam;
			}

			int index = 0; // zmienna przechowuj�ca id studenta otrzymanego z komunikatu
			if (!student_msg.empty() && all_of(student_msg.begin(), student_msg.end(), ::isdigit)) // sprawdzenie poprawno�ci danych komunikatu
				index = stoi(student_msg); // student_msg zawiera index studenta w pami�ci wsp�dzielonej
			else
				handle_error("Nieprawidlowy komunikat");

			Student student = students[index]; // przypisanie studenta do odpowiadaj�cego studenta z pami�ci wsp�dzielonej - listy

			cout << "Komisja A przyjmuje studenta o ID = " << student.id << endl;

			simulate_answers(student, 'A'); // symulujemy zadawanie pyta� i odpowiedzi dla studenta z listy

			// zapisanie wynik�w do pami�ci
			students[index].practic_grade = student.practic_grade;
			students[index].practic_pass = student.practic_pass;

			cout << "Komisja A ocenila studenta o ID = " << student.id << " za praktyke: " << student.practic_grade << endl << endl;

			string result_msg = to_string(index);
			send_msg(msgid_com, 1, result_msg); // wysy�anie do komisji B
		}

		cout << "[" << getpid() << "] Wyslanie studentow do komisji B." << endl;
		
		sem_signal(semid_com, 1); // powiadomienie komisji B, �e max tr�jka student�w jest gotowa
		sem_wait(semid_com, 0); // oczekiwanie na zako�czenie egzaminowania przez komisj� B

		sem_signal(semid_stu, 0); // poinformowanie student�w o wolnej komisji
	}

	end_exam:
	cout << "[" << getpid() << "] Komisja A przeslala wszystkie wyniki do Komisji B." << endl;

	destroy_msg(msgid_stu); // usuni�cie kolejki komunikat�w ze studentami
	destroy_sem(semid_stu); // usuni�cie semafor�w do obs�ugi student�w
	detach_shm(shm_ptr); // od�aczenie wska�nika pami�ci wsp�dzielonej ze studentem, komisj� B oraz dziekanem

	// cleanup(msgid_stu, semid_stu, shm_ptr, shmid_stu); lub z pomoc� funkcji cleanup

	cout << "[" << getpid() << "] Komisja A konczy prace." << endl << endl;

	return 0;
}

void handle_signal(int signum)  // obs�uga czyszczenia zasob�w dla komisji A
{
	usleep(500000);
	send_msg(msgid_com, 1, "END"); // awaryjne powiadomienie komisji B o zako�czeniu egzaminowania
	cout << "Komisja A przerywa proces egzaminu." << endl;
	cleanup(msgid_stu, semid_stu, shm_ptr, -1); // uniwersalna funkcja czyszcz�ca elementy ipc
	cout << "Wyczyszczono zasoby dla komisji A." << endl;
	exit(EXIT_SUCCESS);
}
