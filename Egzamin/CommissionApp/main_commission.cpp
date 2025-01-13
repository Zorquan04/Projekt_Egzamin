#include "ipc.h"
#include "utils.h"
#include "constants.h"
#include "structures.h"
#include "exam_logic.h"

int main()
{
	sleep(1);

	cout << "Komisja gotowa do pracy." << endl;

	sleep(2);

	// tworzenie kolejek komunikat�w oraz semafor�w
	key_t key_stu = generate_key("/home/zorquan/projects/Keyfile", 16); // inicjalizacja kolejki komunikat�w ze studentami
	int msgid_stu = create_msg(key_stu);

	key_t key_dean = generate_key("/home/zorquan/projects/Keyfile", 64); // inicjalizacja kolejki komunikat�w z dziekanem
	int msgid_dean = create_msg(key_dean);

	key_t sem_key = generate_key("/home/zorquan/projects/Keyfile", 128); // klucz do semafor�w
	int semid = create_sem(sem_key, 2); // zestaw semafor�w: 0 - komisja, 1 - studenci
	semctl(semid, 0, SETVAL, 1);

	// odbieranie i przetwarzanie student�w
	vector<Student> students; // lista student�w

	while (true)
	{
		usleep(250000); // argument w mikrosekundach - 0.25s

		sem_wait(semid, 1); // oczekiwanie na powiadomienie od student�w

		string student_msg = receive_msg(msgid_stu, 1); // odbieranie komunikatu o studencie
		if (student_msg == "END") break; // sprawdzenie komunikatu ko�cowego

		cout << "Komisja przyjmuje studenta o ID = " << student_msg << endl;

		Student student; // struktura studenta

		if (!student_msg.empty() && all_of(student_msg.begin(), student_msg.end(), ::isdigit)) // sprawdzenie poprawno�ci danych komunikatu
		{
			student.id = stoi(student_msg); // student_msg zawiera id studenta
		}
		else
		{
			cerr << "Nieprawidlowy komunikat studenta: " << student_msg << endl;
			continue; // pomijamy b��dny komunikat
		}

		students.push_back(student); // dodanie studenta do listy - wektora

		simulate_answers(student); // symulujemy zadawanie pyta� i odpowiedzi

		cout << "Komisja ocenila studenta o ID = " << student.id << endl;
		cout << "Ocena za czesc teoretyczna: " << student.theoric_grade << endl;
		cout << "Ocena za czesc praktyczna: " << student.practic_grade << endl << endl;

		// wystawienie oceny za egzamin
		Exam_result result;
		result.student_id = student.id;
		result.final_grade = calculate_final_grade(student);
		result.passed = student.practic_pass && student.theoric_pass;

		string result_msg = to_string(result.student_id) + " " +
			to_string(result.final_grade) + " " +
			to_string(result.passed);

		send_msg(msgid_dean, 2, result_msg); // wysy�anie wynik�w do dziekana

		sem_signal(semid, 0); // powiadomienie student�w o dost�pno�ci komisji
	}

	sem_signal(semid, 1); // sygna� ko�cowy
	cout << "Komisja przeslala wyniki do dziekana." << endl;
	send_msg(msgid_dean, 2, "END"); // powiadomienie dziekana o zako�czeniu

	destroy_msg(msgid_stu); // usuni�cie kolejki komunikat�w ze studentami
	destroy_sem(semid); // usuni�cie semafor�w do obs�ugi student�w

	cout << "Komisja konczy prace." << endl << endl;

	return 0;
}
