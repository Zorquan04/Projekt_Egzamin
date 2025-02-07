#include "ipc.h"
#include "utils.h"
#include "constants.h"
#include "structures.h"
#include "exam_logic.h"

void handle_signal(int);
void* shm_ptr;
int msgid;

int main(int argc, char* argv[])
{
    signal(SIGINT, handle_signal); // obs³uga sygna³u przerwania
    signal(SIGUSR1, handle_signal); // obs³uga sygna³u awaryjnego
    signal(SIGTSTP, SIG_IGN); // ignoruj sygna³ SIGTSTP (CTRL+Z)

    if (argc < 2)
        handle_error(red("Blad podczas odbierania indeksu studenta"));

    int my_id = atoi(argv[1]); // pobranie wys³anego id z argumentu

    // utworzenie potrzebnych struktur oraz do³¹czenie wskaŸnika do pamiêci wspó³dzielonej
    key_t msg_key = generate_key('A'); // kolejka komunikatów ze wszystkimi
    msgid = create_msg(msg_key);

    key_t shm_key = generate_key('B'); // pamiêæ wspó³dzielona ze wszystkimi
    int shmid = create_shm(shm_key, sizeof(Student) * MAX_STUDENTS);
    shm_ptr = attach_shm(shmid);

    key_t sem_key_stu = generate_key('C'); // semafor ze Studentem
    int semid_stu = create_sem(sem_key_stu, 1);

    key_t sem_key_comA = generate_key('D'); // semafor z komisj¹ A
    int semid_comA = create_sem(sem_key_comA, 1);

    key_t sem_key_start = generate_key('X'); // semafor przepuszczaj¹cy ka¿dego ze studentów
    int semid_start = create_sem(sem_key_start, 2); // domyœlnie zero - brak przejœcia

    // czekamy na sygna³, ¿e filtracja jest zakoñczona (Dziekan przes³a³ kierunek a Student pofiltrowa³ listê i doda³ w pamiêci)
    sem_wait(semid_stu, 0);

    // odnajdywanie siê w pamiêci wspó³dzielonej - liœcie - w celu dowiedzenia siê czy pisze egzamin
    Student* filtered_students = static_cast<Student*>(shm_ptr);
    bool found = false;
    Student my_student;

    for (int i = 0; i < MAX_STUDENTS; i++)
    {
        if (filtered_students[i].id == my_id)
        {
            my_student = filtered_students[i];
            found = true;
            break;
        }
    }

    if (!found) // jeœli student nie znalaz³ siê na liœcie, odchodzi - nie pisze egzaminu
        raise(SIGINT);
        
    sem_wait(semid_start, 0); // czekamy na start od komisji A

    sem_wait(semid_comA, 0); // czekamy, a¿ bêdzie miejsce w komisji A

    // wchodzimy do Komisji A
    string msg = to_string(my_student.id) + "," + to_string(getpid());  // format: "ID,PID"
    send_msg(msgid, 55, msg);
    send_msg(msgid, 5, msg);
    
    // czekamy na wyniki egzaminu i opuszczamy uczelniê
    sem_wait(semid_start, 1);

    detach_shm(shm_ptr); // od³¹czenie wskaŸnika do pamiêci wspó³dzielonej

	return 0;
}

void handle_signal(int signum) // obs³uga czyszczenia zasobów dla studenta
{
    if (signum == SIGINT || signum == SIGUSR1)
    {
        cleanup(-1, -1, -1, shm_ptr, NULL, -1, -1);
        exit(EXIT_SUCCESS);
    }
}