#include "ipc.h"
#include "utils.h"
#include "constants.h"
#include "structures.h"
#include "exam_logic.h"

int main()
{
	cout << "Dziekan uruchamia proces egzaminu." << endl;

	sleep(1);

	// dziekan wybiera kierunek, który bêdzie pisa³ egzamin
	
	// 1. losowanie kierunku
	// srand(static_cast<unsigned int>(time(nullptr))); // inicjalizacja generatora losowego
	// int direction = rand() % 5 + 1; // losowanie kierunku, który podejdzie do egzaminu (od 1 do 5)

	// 2. podanie kierunku przez u¿ytkownika
	int direction = get_direction(); // definicja w utils.cpp
	
	cout << "Dziekan wybral kierunek, ktory podejdzie do egzaminu: " << direction << endl;

	// tworzenie kolejek komunikatów
	key_t key_stu = generate_key("/home/zorquan/projects/Keyfile", 32); // inicjalizacja kolejki komunikatów ze studentami
	int msgid_stu = create_msg(key_stu);

	key_t key_com = generate_key("/home/zorquan/projects/Keyfile", 64); // inicjalizacja kolejki komunikatów z komisj¹
	int msgid_com = create_msg(key_com);

	// wys³anie informacji do studentów
	send_msg(msgid_stu, 3, to_string(direction));
	cout << "Informacja o kierunku wyslana do studentow." << endl;

	cout << "Oczekiwanie na wyniki od komisji..." << endl << endl;

	// pobieranie wyników od komisji
	vector<Exam_result> results;
	while (true) 
	{
		string result_msg = receive_msg(msgid_com, 2); // otrzymanie danych od komisji
		if (result_msg == "END") break; // pobieraj dane a¿ do odnaleziena END symbolizuj¹cego koniec danych

		Exam_result result;
		istringstream iss(result_msg);
		iss >> result.student_id >> result.final_grade >> result.passed; // uzupe³niaj otrzymanymi danymi odpowiednie pola

		if (iss.fail()) 
		{
			handle_error("Blad podczas pobierania wynikow egzaminu");
			continue; // pomijamy b³edne dane
		}

		results.push_back(result); // dodanie elementu do wektora
	}

	cout << "Dziekan otrzymal wyniki od komisji." << endl << endl;
	sleep(2);

	// wypisanie wyników egzaminu
	cout << "Publikowanie wynikow:" << endl;

	int total_students = static_cast<int>(results.size()); // liczba studentów, którzy podeszli do egzaminu
	int passed_students = 0; // liczba zaliczeñ

	for (const auto& result : results) 
	{
		cout << "Student ID: " << result.student_id << ", Ocena: " << result.final_grade
			<< ", Zaliczenie: " << (result.passed ? "Tak" : "Nie") << endl;

		if (result.passed)
			passed_students++; // zliczanie zaliczeñ

		usleep(100000);
	}

	sleep(1);
	cout << endl << "Liczba studentow, ktorzy podeszli do egzaminu: " << total_students << endl;
	cout << "Liczba studentow, ktorzy zdali: " << passed_students << endl;

	sleep(1);
	destroy_msg(msgid_com); // usuniêcie kolejki komunikatów z komisj¹

	cout << endl << "Dziekan konczy proces egzaminu." << endl << endl;

	return 0;
}
