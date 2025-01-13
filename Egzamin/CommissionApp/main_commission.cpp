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

	// tworzenie kolejek komunikatów oraz semaforów
	key_t key_stu = generate_key("/home/zorquan/projects/Keyfile", 16); // inicjalizacja kolejki komunikatów ze studentami
	int msgid_stu = create_msg(key_stu);

	key_t key_dean = generate_key("/home/zorquan/projects/Keyfile", 64); // inicjalizacja kolejki komunikatów z dziekanem
	int msgid_dean = create_msg(key_dean);

	key_t sem_key = generate_key("/home/zorquan/projects/Keyfile", 128); // klucz do semaforów
	int semid = create_sem(sem_key, 2); // zestaw semaforów: 0 - komisja, 1 - studenci
	semctl(semid, 0, SETVAL, 1);

	// odbieranie i przetwarzanie studentów
	vector<Student> students; // lista studentów

	while (true)
	{
		usleep(250000); // argument w mikrosekundach - 0.25s

		sem_wait(semid, 1); // oczekiwanie na powiadomienie od studentów

		string student_msg = receive_msg(msgid_stu, 1); // odbieranie komunikatu o studencie
		if (student_msg == "END") break; // sprawdzenie komunikatu koñcowego

		cout << "Komisja przyjmuje studenta o ID = " << student_msg << endl;

		Student student; // struktura studenta

		if (!student_msg.empty() && all_of(student_msg.begin(), student_msg.end(), ::isdigit)) // sprawdzenie poprawnoœci danych komunikatu
		{
			student.id = stoi(student_msg); // student_msg zawiera id studenta
		}
		else
		{
			cerr << "Nieprawidlowy komunikat studenta: " << student_msg << endl;
			continue; // pomijamy b³êdny komunikat
		}

		students.push_back(student); // dodanie studenta do listy - wektora

		simulate_answers(student); // symulujemy zadawanie pytañ i odpowiedzi

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

		send_msg(msgid_dean, 2, result_msg); // wysy³anie wyników do dziekana

		sem_signal(semid, 0); // powiadomienie studentów o dostêpnoœci komisji
	}

	sem_signal(semid, 1); // sygna³ koñcowy
	cout << "Komisja przeslala wyniki do dziekana." << endl;
	send_msg(msgid_dean, 2, "END"); // powiadomienie dziekana o zakoñczeniu

	destroy_msg(msgid_stu); // usuniêcie kolejki komunikatów ze studentami
	destroy_sem(semid); // usuniêcie semaforów do obs³ugi studentów

	cout << "Komisja konczy prace." << endl << endl;

	return 0;
}
