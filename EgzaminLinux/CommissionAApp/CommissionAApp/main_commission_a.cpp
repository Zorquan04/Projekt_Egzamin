#include "ipc.h"
#include "utils.h"
#include "constants.h"
#include "structures.h"
#include "exam_logic.h"

void handle_signal(int);
int semid_stuX, msgid;
void* shm_ptr;

int main()
{
	signal(SIGINT, handle_signal); // obs�uga sygna�u przerwania
	signal(SIGUSR1, handle_signal); // obs�uga sygna�u awaryjnego

	// tworzenie kolejek komunikat�w, semafor�w oraz pami�ci wsp�dzielonej
	key_t msg_key = generate_key('A'); // inicjalizacja kolejki komunikat�w ze wszystkimi
	msgid = create_msg(msg_key);
	string start = receive_msg(msgid, 30); // czekamy na komunikat do startu
	
	key_t shm_key = generate_key('B'); // inicjalizacja pami�ci wsp�dzielonej ze studentem, komisjami oraz dziekanem
	int shmid = create_shm(shm_key, sizeof(Student) * MAX_STUDENTS); // tworzenie pami�ci wsp�dzielonej
	shm_ptr = attach_shm(shmid); // do��czanie pami�ci wsp�dzielonej

	key_t sem_key_stuX = generate_key('D'); // semafor z ka�dym ze student�w - kolejka max 3 os�b
	semid_stuX = create_sem(sem_key_stuX, 1);
	semctl(semid_stuX, 0, SETVAL, 3);

	key_t sem_key_start = generate_key('X'); // semafor przepuszczajacy ka�dego ze student�w
	int semid_start = create_sem(sem_key_start, 2); // domy�lnie zero - brak przej�cia

	key_t sem_key_comB = generate_key('F'); // semafor z komisj� B
	int semid_comB = create_sem(sem_key_comB, 1);

	cout << yellow("[") << yellow(to_string(getpid())) << yellow("] Komisja A gotowa do pracy.") << endl;

	send_msg(msgid, 20, "START");

	// odbieranie student�w z pami�ci wsp�dzielonej
	Student* students = static_cast<Student*>(shm_ptr);

	// przeprowadzanie cz�ci praktycznej
	string msg = receive_msg(msgid, 15); // odbieramy komunikat z liczb� student�w aby wiedzie� ile razy wys�a� START
	int count = stoi(msg);
	semctl(semid_start, 0, SETVAL, count); // ustawienie semafora na liczb� student�w aby ka�dy m�g� wej��

	while (true)
	{
		// egzaminowanie tr�jki student�w
		string student_msg = receive_msg(msgid, 55); // odbieranie komunikatu o studentach
		if (student_msg == "END") // sprawdzenie czy ju� komunikat ko�cowy
		{
			send_msg(msgid, 25, "END"); // powiadomienie komisji B o zako�czeniu pracy ze studentami
			break;
		}

		// parsowanie wiadomo�ci na ID i PID
		size_t delimiter = student_msg.find(',');
		if (delimiter == string::npos)
			handle_error(red("Nieprawidlowy format komunikatu"));

		string id_part = student_msg.substr(0, delimiter);
		string pid_part = student_msg.substr(delimiter + 1);

		int id = stoi(id_part);
		int pid = stoi(pid_part);

		// wyszukujemy studenta w pami�ci wsp�dzielonej na podstawie jego ID
		int found_index = -1;
		for (int i = 0; i < MAX_STUDENTS; i++)
		{
			if (students[i].id == id)
			{
				found_index = i;
				break;
			}
		}
		if (found_index == -1)
			handle_error(red("Nie znaleziono studenta o podanym ID w pamieci."));

		Student student = students[found_index]; // przypisanie studenta do odpowiadaj�cego studenta z pami�ci wsp�dzielonej - listy

		cout << yellow("Komisja A przyjmuje studenta [") << yellow(to_string(pid)) << yellow("] o ID = ") << student.id << endl;

		int semnum = semctl(semid_stuX, 0, GETVAL);

		simulate_answers(student, 'A', pid, semnum); // symulujemy zadawanie pyta� i odpowiedzi dla studenta z listy	

		// zapisanie wynik�w do pami�ci
		students[found_index].practic_grade = student.practic_grade;
		students[found_index].practic_pass = student.practic_pass;

		cout << yellow("Komisja A ocenila studenta [") << yellow(to_string(pid)) << yellow("] o ID = ") << student.id
			<< yellow(" za praktyke: ") << student.practic_grade << endl << endl;

		sem_wait(semid_comB, 0); // oczekiwanie na wolne miejsce w komisji B

		string msg = to_string(id) + "," + to_string(pid);
		send_msg(msgid, 25, msg); // wysy�anie do komisji B

		sem_signal(semid_stuX, 0); // poinformowanie student�w o wolnej komisji
	}

	destroy_sem(semid_stuX); // usuni�cie semafor�w dla ka�dego ze student�w
	detach_shm(shm_ptr); // od�aczenie wska�nika pami�ci wsp�dzielonej ze studentem, komisjami oraz dziekanem

	cout << yellow("[") << yellow(to_string(getpid())) << yellow("] Komisja A konczy prace.") << endl << endl;

	return 0;
}

void handle_signal(int signum)  // obs�uga czyszczenia zasob�w dla komisji A
{
	if (signum == SIGINT)
	{
		cout << yellow("Komisja A przerywa proces egzaminu.") << endl;

		cleanup(-1, semid_stuX, -1, shm_ptr, NULL, -1, -1); // uniwersalna funkcja czyszcz�ca elementy ipc

		cout << yellow("Wyczyszczono zasoby dla komisji A.") << endl;
		exit(EXIT_SUCCESS);
	}

	if (signum == SIGUSR1)
	{
		// send_msg(msgid, 60, "END"); // wys�anie awaryjnego komunikatu do ka�dego ze student�w
		cout << yellow("Komisja A przerywa proces egzaminu.") << endl;

		cleanup(-1, semid_stuX, -1, shm_ptr, NULL, -1, -1); // uniwersalna funkcja czyszcz�ca elementy ipc

		cout << yellow("Wyczyszczono zasoby dla komisji A.") << endl;
		exit(EXIT_SUCCESS);
	}
}
