#include "ipc.h"
#include "utils.h"
#include "constants.h"
#include "structures.h"
#include "exam_logic.h"

void handle_signal(int);
int msgid_dean;
void* shm_ptr;
Student* all_students;

int main()
{
    signal(SIGINT, handle_signal); // obs³uga sygna³u awaryjnego

    // tworzenie kolejki komunikatów, semaforów oraz pamiêci wspó³dzielonej
    key_t key_com = generate_key('A'); // inicjalizacja kolejki komunikatów z komisj¹
    int msgid_com = create_msg(key_com);

    key_t key_dean = generate_key('B'); // inicjalizacja kolejki komunikatów z dziekanem
    msgid_dean = create_msg(key_dean);

    key_t sem_key_com = generate_key('D'); // klucz do semaforów z komisj¹
    int semid_com = create_sem(sem_key_com, 2); // tworzenie zestawu 2 semaforów: 0 - komisja, 1 - studenci

    key_t sem_key_dean = generate_key('E'); // klucz do semaforów z dziekanem
    int semid_dean = create_sem(sem_key_dean, 1); // tworzymy jeden semafor dla sygnalizacji gotowoœci dla dziekana

    key_t shm_key = generate_key('F'); // inicjalizacja pamiêci wspó³dzielonej z komisj¹
    int shmid = create_shm(shm_key, sizeof(Student) * MAX_STUDENTS); // tworzenie pamiêci wspó³dzielonej
    shm_ptr = attach_shm(shmid); // do³¹czanie pamiêci wspó³dzielonej

    sleep(2);
    cout << "[" << getpid() << "] Rozpoczecie przybywania studentow na miejsce..." << endl << endl;
    sleep(1);

    // wygenerowanie wszystkich studentów
    all_students = new Student[MAX_STUDENTS * 5]; // stworzenie tablicy przechowuj¹cej wszystkich studentów czekaj¹cych przed uczelni¹
    generate_students(NUM_DIRECTIONS, MIN_STUDENTS, MAX_STUDENTS, all_students);

    cout << endl << "[" << getpid() << "] Wszyscy studenci przybyli na egzamin." << endl << endl;
    sleep(1);

    // liczenie studentów, którzy maj¹ ju¿ zdan¹ praktykê
    int passed_practic = 0;
    int all_student = 0;
    for (int i = 0; i < MAX_STUDENTS * 5; ++i)
    {
        if (all_students[i].practic_pass)
            ++passed_practic;
        all_student++;
    }

    cout << "Liczba studentow, ktorzy powtarzaja egzamin: " << passed_practic << " (" << (100.0 * passed_practic / all_student) << "%)" << endl << endl;

    usleep(500000);
    sem_signal(semid_dean, 0); // sygnalizacja gotowoœci do dziekana
    cout << "[" << getpid() << "] Oczekiwanie na informacje od dziekana..." << endl;

    // odebranie informacji od dziekana na temat kierunku, który podchodzi do egzaminu
    string direction_msg = receive_msg(msgid_dean, 3); // oczekiwanie na informacjê od dziekana na temat kierunku
    sleep(1);
    cout << "[" << getpid() << "] Studenci dowiedzieli sie, ktory kierunek pisze egzamin." << endl << endl;
    int target_direction = stoi(direction_msg); // konwersja string na int

    sleep(1);

    // filtrowanie studentów wed³ug kierunku - lista studentów przystêpuj¹cych do egzaminu
    Student* filtered_students = static_cast<Student*>(shm_ptr); // rzutowanie wskaŸnika do struktury Student - tablica z przefiltrowanymi studentami
    int index = 0;
    for (int i = 0; i < MAX_STUDENTS * 5; ++i)
    {
        if (all_students[i].direction == target_direction)
        {
            filtered_students[index] = all_students[i]; // jeœli student jest z odpowiedniego kierunku zostaje dodany do listy egzaminacyjnej
            ++index;
        }
    }

    cout << "[" << getpid() << "] Lista ID studentow podchodzacych do egzaminu:" << endl;
    for (int i = 0; i < index; ++i)
        cout << filtered_students[i].id << ", ";

    sleep(1);
    cout << endl << endl << "[" << getpid() << "] Rozpoczecie przeprowadzania egzaminu..." << endl << endl;
    sleep(1);

    // wysy³anie studentów do komisji
    for (int i = 0; i < index; ++i)
    {
        sem_wait(semid_com, 0); // oczekiwanie na dostêp komisji

        string student_msg = to_string(filtered_students[i].id);
        send_msg(msgid_com, 1, student_msg); // wysy³anie komunikatu o dostêpnoœci konkretnego studenta
        cout << "Student o ID = " << filtered_students[i].id << " wyslany do komisji." << endl;

        sem_signal(semid_com, 1); // powiadomienie komisji o nowym studencie
    }

    sem_wait(semid_com, 0); // upewnienie ¿e komisja zakoñczy³a pracê z ostatnim studentem
    send_msg(msgid_com, 1, "END"); // wys³anie komunikatu zakoñczenia procesu

    destroy_msg(msgid_dean); // usuniêcie kolejki komunikatów z dziekanem
    detach_shm(shm_ptr); // od³¹czenie wskaŸnika pamiêci wspó³dzielonej z komisj¹
    delete[] all_students; // usuniêcie tabeli zawieraj¹cej wszystkich studentów, którzy pojawili siê przed uczelni¹

    // cleanup(msgid_dean, -1, shm_ptr, NULL, -1); lub z pomoc¹ funkcji cleanup

    cout << endl << "[" << getpid() << "] Wszyscy studenci zakonczyli egzamin." << endl << endl;

    return 0;
}

void handle_signal(int signum) // obs³uga czyszczenia zasobów dla studenta
{
    cout << endl << "Student przerywa proces egzaminu." << endl;
    cleanup(msgid_dean, -1, shm_ptr, NULL, -1); // uniwersalna funkcja czyszcz¹ca elementy ipc
    delete[] all_students;
    cout << "Wyczyszczono zasoby dla studenta." << endl;
    exit(EXIT_SUCCESS);
}
