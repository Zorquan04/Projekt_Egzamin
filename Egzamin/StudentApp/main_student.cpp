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
    signal(SIGINT, handle_signal); // obs�uga sygna�u awaryjnego

    // tworzenie kolejki komunikat�w, semafor�w oraz pami�ci wsp�dzielonej
    key_t msg_key_dean = generate_key('A'); // inicjalizacja kolejki komunikat�w z dziekanem
    msgid_dean = create_msg(msg_key_dean);

    key_t msg_key_com = generate_key('B'); // inicjalizacja kolejki komunikat�w z komisj� A
    int msgid_com = create_msg(msg_key_com);

    key_t sem_key_dean = generate_key('E'); // klucz do semafor�w z dziekanem
    int semid_dean = create_sem(sem_key_dean, 1); // tworzymy jeden semafor dla sygnalizacji gotowo�ci dla dziekana

    key_t sem_key_com = generate_key('F'); // klucz do semafor�w z komisj� A
    int semid_com = create_sem(sem_key_com, 2); // tworzenie zestawu 2 semafor�w: 0 - komisja, 1 - studenci

    key_t shm_key = generate_key('H'); // inicjalizacja pami�ci wsp�dzielonej z komisj� i dziekanem
    int shmid = create_shm(shm_key, sizeof(Student) * MAX_STUDENTS); // tworzenie pami�ci wsp�dzielonej
    shm_ptr = attach_shm(shmid); // do��czanie pami�ci wsp�dzielonej

    sleep(2);
    cout << "[" << getpid() << "] Rozpoczecie przybywania studentow na miejsce..." << endl << endl;
    sleep(1);

    // wygenerowanie wszystkich student�w
    all_students = new Student[MAX_STUDENTS * 5]; // stworzenie tablicy przechowuj�cej wszystkich student�w czekaj�cych przed uczelni�
    generate_students(NUM_DIRECTIONS, MIN_STUDENTS, MAX_STUDENTS, all_students);

    cout << endl << "[" << getpid() << "] Wszyscy studenci przybyli na egzamin." << endl << endl;
    sleep(1);

    // liczenie student�w, kt�rzy maj� ju� zdan� praktyk�
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
    sem_signal(semid_dean, 0); // sygnalizacja gotowo�ci do dziekana
    cout << "[" << getpid() << "] Oczekiwanie na informacje od dziekana..." << endl;

    // odebranie informacji od dziekana na temat kierunku, kt�ry podchodzi do egzaminu
    string direction_msg = receive_msg(msgid_dean, 1); // oczekiwanie na informacj� od dziekana na temat kierunku
    sleep(1);
    cout << "[" << getpid() << "] Studenci dowiedzieli sie, ktory kierunek pisze egzamin." << endl << endl;
    int target_direction = stoi(direction_msg); // konwersja string na int

    sleep(1);

    // filtrowanie student�w wed�ug kierunku - lista student�w przyst�puj�cych do egzaminu
    Student* filtered_students = static_cast<Student*>(shm_ptr); // rzutowanie wska�nika do struktury Student - tablica z przefiltrowanymi studentami
    int total = 0;
    for (int i = 0; i < MAX_STUDENTS * 5; ++i)
    {
        if (all_students[i].direction == target_direction)
        {
            filtered_students[total] = all_students[i]; // je�li student jest z odpowiedniego kierunku zostaje dodany do listy egzaminacyjnej
            ++total;
        }
    }

    cout << "[" << getpid() << "] Lista ID studentow podchodzacych do egzaminu:" << endl;
    for (int i = 0; i < total; ++i)
    {
        cout << filtered_students[i].id;
        if (i == total - 1)
            cout << ".";
        else
            cout << ", ";
    }

    sleep(1);
    cout << endl << endl << "[" << getpid() << "] Rozpoczecie przeprowadzania egzaminu..." << endl << endl;
    sleep(1);

    // wysy�anie student�w do komisji A
    for (int i = 0; i < total; i += 3)
    {
        sem_wait(semid_com, 0); // oczekiwanie na dost�p komisji A

        for (int j = 0; j < 3 && (i + j) < total; ++j)
        {
            string student_msg = to_string(i + j);
            send_msg(msgid_com, 1, student_msg); // wysy�anie komunikatu o dost�pno�ci konkretnego studenta
            cout << "Student o ID = " << filtered_students[i + j].id << " wyslany do komisji A." << endl;
        }
        cout << endl;
        sem_signal(semid_com, 1); // powiadomienie komisji A o nowej tr�jce    
    }

    //sem_wait(semid_com, 0); // upewnienie �e komisja zako�czy�a prac� z ostatnim studentem
    send_msg(msgid_com, 1, "END"); // wys�anie komunikatu zako�czenia procesu
    sem_signal(semid_com, 1); // ostatni sygna� dla komisji do odebrania END

    destroy_msg(msgid_dean); // usuni�cie kolejki komunikat�w z dziekanem
    detach_shm(shm_ptr); // od��czenie wska�nika pami�ci wsp�dzielonej z komisj� A
    delete[] all_students; // usuni�cie tabeli zawieraj�cej wszystkich student�w, kt�rzy pojawili si� przed uczelni�

    // cleanup(msgid_dean, -1, shm_ptr, -1); lub z pomoc� funkcji cleanup

    cout << endl << "[" << getpid() << "] Wszyscy studenci zostali wyslani na egzamin." << endl << endl;

    return 0;
}

void handle_signal(int signum) // obs�uga czyszczenia zasob�w dla studenta
{
    usleep(250000);
    cout << "Student przerywa proces egzaminu." << endl;
    cleanup(msgid_dean, -1, shm_ptr, -1); // uniwersalna funkcja czyszcz�ca elementy ipc
    if (all_students)
        delete[] all_students;
    cout << "Wyczyszczono zasoby dla studenta." << endl;
    exit(EXIT_SUCCESS);
}
