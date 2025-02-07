#include "ipc.h"
#include "utils.h"
#include "constants.h"
#include "structures.h"
#include "exam_logic.h"

void handle_signal(int);
int semid_comA, msgid;
void* shm_ptr;

int main()
{
    signal(SIGINT, handle_signal); // obs�uga sygna�u przerwania
    signal(SIGUSR1, handle_signal); // obs�uga sygna�u awaryjnego

    // tworzenie kolejek komunikat�w, semafor�w oraz pami�ci wsp�dzielonej
    key_t msg_key = generate_key('A'); // inicjalizacja kolejki komunikat�w ze wszystkimi
    msgid = create_msg(msg_key);
    string start = receive_msg(msgid, 40); // czekamy na komunikat do startu
    
    key_t shm_key = generate_key('B'); // inicjalizacja pami�ci wsp�dzielonej z komisjami, studentami oraz dziekanem
    int shmid = create_shm(shm_key, sizeof(Student) * MAX_STUDENTS); // tworzenie pami�ci wsp�dzielonej
    shm_ptr = attach_shm(shmid); // do��czanie pami�ci wsp�dzielonej

    key_t sem_key_comA = generate_key('F'); // semafor z komisj� A - kolejka max 3 os�b
    semid_comA = create_sem(sem_key_comA, 1);
    semctl(semid_comA, 0, SETVAL, 3);

    cout << blue("[") << blue(to_string(getpid())) << blue("] Komisja B gotowa do pracy.") << endl;

    send_msg(msgid, 30, "START");

    // odbieranie student�w z pami�ci wsp�dzielonej
    Student* students = static_cast<Student*>(shm_ptr);

    // przeprowadzanie cz�ci teoretycznej
    while (true)
    {
        // egzaminowanie tr�jki student�w
        string student_msg = receive_msg(msgid, 25); // odbieranie komunikatu o studentach
        if (student_msg == "END") // sprawdzenie czy ju� komunikat ko�cowy
        {
            send_msg(msgid, 35, "END"); // powiadomienie komisji B o zako�czeniu pracy ze studentami
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

        // szukamy studenta w pami�ci wsp�dzielonej � przeszukujemy ca�� tablic�, aby znale�� rekord z danym ID
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
            handle_error(red("Nie znaleziono studenta o podanym ID w pamieci wspoldzielonej"));

        Student student = students[found_index]; // przypisanie studenta do odpowiadaj�cego studenta z pami�ci, ktory zdal praktyke

        cout << blue("Komisja B przyjmuje studenta [") << blue(to_string(pid)) << blue("] o ID = ") << student.id << endl;

        int semnum = semctl(semid_comA, 0, GETVAL);

        simulate_answers(student, 'B', pid, semnum); // symulujemy zadawanie pyta� i odpowiedzi dla studenta z listy

        // zapisanie wynik�w do pami�ci
        students[found_index].theoric_grade = student.theoric_grade;
        students[found_index].theoric_pass = student.theoric_pass;

        cout << blue("Komisja B ocenila studenta [") << blue(to_string(pid)) << blue("] o ID = ") << student.id << blue(" za teorie: ") << student.theoric_grade << endl << endl;

        send_msg(msgid, 35, to_string(student.id)); // wysy�anie informacji o studencie ko�cz�cym egzamin do dziekana
        
        sem_signal(semid_comA, 0); // odblokowanie komisji B dla kolejnych student�w
    }

    send_msg(msgid, 35, "END"); // powiadomienie dziekana o zako�czeniu egzaminu
    cout << blue("[") << blue(to_string(getpid())) << blue("] Komisja B przeslala wyniki do dziekana.") << endl;

    destroy_sem(semid_comA); // usuni�cie semafor�w do obs�ugi komisji A
    detach_shm(shm_ptr); // od�aczenie wska�nika pami�ci wsp�dzielonej z komisjami, studentami oraz dziekanem

	cout << blue("[") << blue(to_string(getpid())) << blue("] Komisja B konczy prace.") << endl << endl;

	return 0;
}

void handle_signal(int signum)  // obs�uga czyszczenia zasob�w dla komisji
{
    if (signum == SIGINT)
    {
        cout << blue("Komisja B przerywa proces egzaminu.") << endl;

        cleanup(-1, semid_comA, -1, shm_ptr, NULL, -1, -1); // uniwersalna funkcja czyszcz�ca elementy ipc

        cout << blue("Wyczyszczono zasoby dla komisji B.") << endl;
        exit(EXIT_SUCCESS);
    }

    if (signum == SIGUSR1)
    {
        send_msg(msgid, 35, "END"); // awaryjne powiadomienie dziekana o zako�czeniu egzaminowania
        cout << blue("Komisja B przerywa proces egzaminu.") << endl;

        cleanup(-1, semid_comA, -1, shm_ptr, NULL, -1, -1); // uniwersalna funkcja czyszcz�ca elementy ipc

        cout << blue("Wyczyszczono zasoby dla komisji B.") << endl;
        exit(EXIT_SUCCESS);
    }
}