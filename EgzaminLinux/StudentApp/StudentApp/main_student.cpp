#include "ipc.h"
#include "utils.h"
#include "constants.h"
#include "structures.h"
#include "exam_logic.h"

void handle_signal(int);
int semid_stuX, msgid;
void* shm_ptr;
void* shm_ptr_pid;
Student* all_students;

int main()
{
    signal(SIGINT, handle_signal); // obs�uga sygna�u przerwania
    signal(SIGUSR1, handle_signal); // obs�uga sygna�u awaryjnego
    signal(SIGCHLD, handle_signal); // zbieranie zako�czonych proces�w potomnych

    // tworzenie kolejki komunikat�w, semafor�w oraz pami�ci wsp�dzielonej
    key_t msg_key = generate_key('A'); // inicjalizacja kolejki komunikat�w ze wszystkimi
    msgid = create_msg(msg_key);
    string start = receive_msg(msgid, 20); // czekamy na komunikat do startu    

    key_t shm_key = generate_key('B'); // inicjalizacja pami�ci wsp�dzielonej ze wszystkimi
    int shmid = create_shm(shm_key, sizeof(Student) * MAX_STUDENTS); // tworzenie pami�ci wsp�dzielonej
    shm_ptr = attach_shm(shmid); // do��czanie pami�ci wsp�dzielonej

    key_t sem_key_stuX = generate_key('C'); // klucz do semafor�w z ka�dym ze student�w
    semid_stuX = create_sem(sem_key_stuX, 1); // tworzymy jeden semafor dla sygnalizacji gotowo�ci ze studentami

    key_t sem_key_dean = generate_key('E'); // klucz do semafor�w z dziekanem
    int semid_dean = create_sem(sem_key_dean, 1); // tworzymy jeden semafor dla sygnalizacji gotowo�ci dla dziekana

    key_t shm_key_pid = generate_key('Y'); // pomocnicza pami�� wsp�dzielona z pidami student�w
    int shmid_pid = create_shm(shm_key_pid, sizeof(Student) * MAX_STUDENTS);
    shm_ptr_pid = attach_shm(shmid_pid);

    cout << green("[") << green(to_string(getpid())) << green("] Rozpoczecie przybywania studentow na miejsce...") << endl << endl;

    // wygenerowanie wszystkich student�w
    all_students = new Student[MAX_STUDENTS * NUM_DIRECTIONS]; // stworzenie tablicy przechowuj�cej wszystkich student�w czekaj�cych przed uczelni�
    vector<pid_t> child_pids = generate_students(NUM_DIRECTIONS, MIN_STUDENTS, MAX_STUDENTS, all_students);
    send_msg(msgid, 15, to_string(child_pids.size())); // wysy�amy liczb� student�w do Komisji A i dziekana
    send_msg(msgid, 50, to_string(child_pids.size()));
    memcpy(shm_ptr_pid, child_pids.data(), child_pids.size() * sizeof(pid_t)); // wysy�amy pidy student�w do dziekana w postaci wektora

    cout << endl << green("[") << green(to_string(getpid())) << green("] Wszyscy studenci przybyli na egzamin.") << endl << endl;

    // liczenie student�w, kt�rzy maj� ju� zdan� praktyk�
    int passed_practic = 0;
    int total_students = 0;
    for (int i = 0; i < MAX_STUDENTS * NUM_DIRECTIONS; ++i)
    {
        if (all_students[i].practic_pass)
            ++passed_practic;
        total_students++;
    }

    cout << green("Liczba studentow, ktorzy powtarzaja egzamin: ") << passed_practic << green(" (")
        << (100.0 * passed_practic / total_students) << green("%)") << endl << endl;

    sem_signal(semid_dean, 0); // sygnalizacja gotowo�ci do dziekana
    cout << green("[") << green(to_string(getpid())) << green("] Oczekiwanie na informacje od dziekana...") << endl;

    // odebranie informacji od dziekana na temat kierunku, kt�ry podchodzi do egzaminu
    string direction_msg = receive_msg(msgid, 50); // oczekiwanie na informacj� od dziekana na temat kierunku
    cout << green("[") << green(to_string(getpid())) << green("] Studenci dowiedzieli sie, ktory kierunek pisze egzamin.") << endl << endl;
    int target_direction = stoi(direction_msg); // konwersja string na int

    // filtrowanie student�w wed�ug kierunku - lista student�w przyst�puj�cych do egzaminu
    Student* filtered_students = static_cast<Student*>(shm_ptr); // tablica z przefiltrowanymi studentami
    int total = 0;
    for (int i = 0; i < MAX_STUDENTS * NUM_DIRECTIONS; ++i)
    {
        if (all_students[i].direction == target_direction)
        {
            filtered_students[total] = all_students[i]; // je�li student jest z odpowiedniego kierunku zostaje dodany do listy egzaminacyjnej
            ++total;
        }
    }

    cout << green("[") << green(to_string(getpid())) << green("] Lista ID studentow podchodzacych do egzaminu:") << endl;
    for (int i = 0; i < total; ++i)
    {
        cout << filtered_students[i].id;
        if (i == total - 1)
            cout << green(".");
        else
            cout << green(", ");
    }

    cout << endl << endl << green("[") << green(to_string(getpid())) << green("] Rozpoczecie przeprowadzania egzaminu...") << endl << endl;

    // zasygnalizowanie ka�demu studentowi gotowej tablicy-listy w pami�ci
    for (size_t i = 0; i < child_pids.size(); i++)
        sem_signal(semid_stuX, 0);

    int count = 0;
    while (count != total) // przyjmujemy komunikaty od przyj�tych student�w do komisji i je zliczamy
    {
        string received_id = receive_msg(msgid, 5);
        count++;
    }

    send_msg(msgid, 55, "END"); // wys�anie komunikatu zako�czenia procesu

    detach_shm(shm_ptr); // od��czenie wska�nika pami�ci wsp�dzielonej ze wszystkimi
    detach_shm(shm_ptr_pid); // od�aczenie wska�nika pami�ci z dziekanem
    destroy_sem(semid_stuX); // usuni�cie semafora ze studentami
    if (all_students)
        delete[] all_students; // usuni�cie tabeli zawieraj�cej wszystkich student�w, kt�rzy pojawili si� przed uczelni�

    // czekamy na zako�czenie wszystkich proces�w studenta
    for (pid_t pid : child_pids)
    {
        if (waitpid(pid, NULL, 0) == -1)
        {
            if (errno != ECHILD)
                handle_error("Blad podczas oczekiwania na zakonczenie procesu studenta");
        }
    }

    cout << endl << green("[") << green(to_string(getpid())) << green("] Wszyscy studenci poznali wyniki egzaminu i zakonczyli.") << endl << endl;

    return 0;
}

void handle_signal(int signum) // obs�uga czyszczenia zasob�w dla studenta
{
    if (signum == SIGUSR1 || signum == SIGINT)
    {
        cout << green("Student przerywa proces egzaminu.") << endl;

        cleanup(-1, semid_stuX, -1, shm_ptr, NULL, -1, -1); // uniwersalna funkcja czyszcz�ca elementy ipc
        detach_shm(shm_ptr_pid);
        if (all_students)
            delete[] all_students;

        cout << green("Wyczyszczono zasoby dla studenta.") << endl;
        exit(EXIT_SUCCESS);
    }
    if (signum == SIGCHLD)
    {
        (void)signum;
        pid_t pid;
        int status;
        while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {}
    }
}
