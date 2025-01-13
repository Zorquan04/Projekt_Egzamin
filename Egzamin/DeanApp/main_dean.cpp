#include "ipc.h"
#include "utils.h"
#include "constants.h"
#include "structures.h"
#include "exam_logic.h"

int main()
{
	cout << "Dziekan uruchamia proces egzaminu." << endl;

	sleep(3);

	// dziekan podaje kierunek, kt�ry b�dzie pisa� egzamin
	srand(static_cast<unsigned int>(time(nullptr))); // inicjalizacja generatora losowego
	int direction = rand() % 5 + 1; // losowanie kierunku, kt�ry podejdzie do egzaminu (od 1 do 5)
	cout << "Dziekan wybral kierunek, ktory podejdzie do egzaminu: " << direction << endl;

	// tworzenie kolejek komunikat�w
	key_t key_stu = generate_key("/home/zorquan/projects/Keyfile", 32); // inicjalizacja kolejki komunikat�w ze studentami
	int msgid_stu = create_msg(key_stu);

	key_t key_com = generate_key("/home/zorquan/projects/Keyfile", 64); // inicjalizacja kolejki komunikat�w z komisj�
	int msgid_com = create_msg(key_com);

	// wys�anie informacji do student�w
	send_msg(msgid_stu, 3, to_string(direction));
	cout << "Informacja o kierunku wyslana do studentow." << endl;

	cout << "Oczekiwanie na wyniki od komisji..." << endl << endl;

	// pobieranie wynik�w od komisji
	vector<Exam_result> results;
	while (true) 
	{
		string result_msg = receive_msg(msgid_com, 2); // otrzymanie danych od komisji
		if (result_msg == "END") break; // pobieraj dane a� do odnaleziena END symbolizuj�cego koniec danych

		Exam_result result;
		istringstream iss(result_msg);
		iss >> result.student_id >> result.final_grade >> result.passed; // uzupe�niaj otrzymanymi danymi odpowiednie zmienne

		if (iss.fail()) 
		{
			cerr << "Blad podczas pobierania wynikow egzaminu: " << result_msg << endl;
			continue;
		}

		results.push_back(result); // dodanie elementu do wektora
	}

	cout << "Dziekan otrzymal wyniki od komisji." << endl << endl;
	sleep(1);

	// wypisanie wynik�w egzaminu
	cout << "Publikowanie wynikow:" << endl;
	for (const auto& result : results) 
	{
		cout << "Student ID: " << result.student_id << ", Ocena: " << result.final_grade
			<< ", Zaliczenie: " << (result.passed ? "Tak" : "Nie") << endl;
	}

	destroy_msg(msgid_com); // usuni�cie kolejki komunikat�w z komisj�

	cout << "Dziekan konczy proces egzaminu." << endl << endl;

	return 0;
}
