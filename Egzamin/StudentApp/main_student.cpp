#include "ipc.h"
#include "utils.h"
#include "constants.h"
#include "structures.h"
#include "exam_logic.h"

int main()
{
    // wygenerowanie wszystkich student�w
	vector<Student> students; // wektor przechowuj�cy wszystkich student�w - lista
	generate_students(NUM_DIRECTIONS, MIN_STUDENTS, MAX_STUDENTS, students);

    sleep(2);

    cout << "Studenci przybyli na egzamin." << endl << endl;

    // tworzenie kolejki komunikat�w oraz semafor�w
    key_t key_com = generate_key("/home/zorquan/projects/Keyfile", 16); // inicjalizacja kolejki komunikat�w z komisj�
    int msgid_com = create_msg(key_com);

    key_t key_dean = generate_key("/home/zorquan/projects/Keyfile", 32); // inicjalizacja kolejki komunikat�w z dziekanem
    int msgid_dean = create_msg(key_dean);

    key_t sem_key = generate_key("/home/zorquan/projects/Keyfile", 128); // klucz do semafor�w
    int semid = create_sem(sem_key, 2); // tworzenie zestawu 2 semafor�w: 0 - komisja, 1 - studenci

    // odebranie informacji od dziekana na temat kierunku, kt�ry podchodzi do egzaminu
    string direction_msg = receive_msg(msgid_dean, 3); // oczekiwanie na informacj� od dziekana na temat kierunku
    usleep(100000);
    cout << "Studenci dowiedzieli sie, ktory kierunek pisze egzamin." << endl;
    int target_direction = stoi(direction_msg);

    sleep(1);

    // filtrowanie student�w wed�ug kierunku - lista student�w przyst�puj�cych do egzaminu
    vector<Student> filtered_students;
    for (const auto& student : students)
    {
        if (student.direction == target_direction)
            filtered_students.push_back(student); // je�li student jest z odpowiedniego kierunku zostaje dodany do listy egzaminacyjnej
    }

    cout << "Lista ID studentow podchodzacych do egzaminu:" << endl;
    for (const auto& student : filtered_students)
        cout << student.id << endl;

    cout << endl;
    sleep(1);

    // wysy�anie student�w do komisji
    for (const auto& student : filtered_students)
    {
        sem_wait(semid, 0); // oczekiwanie na dost�p komisji

        string student_msg = to_string(student.id);
        send_msg(msgid_com, 1, student_msg); // wysy�anie komunikatu o dost�pno�ci studenta
        cout << "Student o ID = " << student.id << " wyslany do komisji." << endl;

        sem_signal(semid, 1); // powiadomienie komisji o nowym studencie
    }

    sem_wait(semid, 0); // upewnienie �e komisja zako�czy�a prac� z ostatnim studentem
    send_msg(msgid_com, 1, "END"); // wys�anie komunikatu zako�czenia procesu
    sem_signal(semid, 1); // ostatni sygna� dla komisji

    destroy_msg(msgid_dean); // usuni�cie kolejki komunikat�w z dziekanem

    cout << "Wszyscy studenci zakonczyli egzamin." << endl << endl;;

	return 0;
}
