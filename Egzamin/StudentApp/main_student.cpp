#include "ipc.h"
#include "utils.h"
#include "constants.h"
#include "structures.h"
#include "exam_logic.h"

int main()
{
    // wygenerowanie wszystkich studentów
	vector<Student> students; // wektor przechowuj¹cy wszystkich studentów - lista
	generate_students(NUM_DIRECTIONS, MIN_STUDENTS, MAX_STUDENTS, students);

    sleep(2);

    cout << "Studenci przybyli na egzamin." << endl << endl;

    // tworzenie kolejki komunikatów oraz semaforów
    key_t key_com = generate_key("/home/zorquan/projects/Keyfile", 16); // inicjalizacja kolejki komunikatów z komisj¹
    int msgid_com = create_msg(key_com);

    key_t key_dean = generate_key("/home/zorquan/projects/Keyfile", 32); // inicjalizacja kolejki komunikatów z dziekanem
    int msgid_dean = create_msg(key_dean);

    key_t sem_key = generate_key("/home/zorquan/projects/Keyfile", 128); // klucz do semaforów
    int semid = create_sem(sem_key, 2); // tworzenie zestawu 2 semaforów: 0 - komisja, 1 - studenci

    // odebranie informacji od dziekana na temat kierunku, który podchodzi do egzaminu
    string direction_msg = receive_msg(msgid_dean, 3); // oczekiwanie na informacjê od dziekana na temat kierunku
    usleep(100000);
    cout << "Studenci dowiedzieli sie, ktory kierunek pisze egzamin." << endl;
    int target_direction = stoi(direction_msg);

    sleep(1);

    // filtrowanie studentów wed³ug kierunku - lista studentów przystêpuj¹cych do egzaminu
    vector<Student> filtered_students;
    for (const auto& student : students)
    {
        if (student.direction == target_direction)
            filtered_students.push_back(student); // jeœli student jest z odpowiedniego kierunku zostaje dodany do listy egzaminacyjnej
    }

    cout << "Lista ID studentow podchodzacych do egzaminu:" << endl;
    for (const auto& student : filtered_students)
        cout << student.id << endl;

    cout << endl;
    sleep(1);

    // wysy³anie studentów do komisji
    for (const auto& student : filtered_students)
    {
        sem_wait(semid, 0); // oczekiwanie na dostêp komisji

        string student_msg = to_string(student.id);
        send_msg(msgid_com, 1, student_msg); // wysy³anie komunikatu o dostêpnoœci studenta
        cout << "Student o ID = " << student.id << " wyslany do komisji." << endl;

        sem_signal(semid, 1); // powiadomienie komisji o nowym studencie
    }

    sem_wait(semid, 0); // upewnienie ¿e komisja zakoñczy³a pracê z ostatnim studentem
    send_msg(msgid_com, 1, "END"); // wys³anie komunikatu zakoñczenia procesu
    sem_signal(semid, 1); // ostatni sygna³ dla komisji

    destroy_msg(msgid_dean); // usuniêcie kolejki komunikatów z dziekanem

    cout << "Wszyscy studenci zakonczyli egzamin." << endl << endl;;

	return 0;
}
