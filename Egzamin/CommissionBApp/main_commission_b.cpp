#include "ipc.h"
#include "utils.h"
#include "constants.h"
#include "structures.h"
#include "exam_logic.h"

void handle_signal(int);
int msgid_dean, msgid_com, semid, shmid;
void* shm_ptr;

int main()
{
    signal(SIGINT, handle_signal); // obs³uga sygna³u awaryjnego

    // tworzenie kolejek komunikatów, semaforów oraz pamiêci wspó³dzielonej
    key_t msg_key_com = generate_key('C'); // inicjalizacja kolejki komunikatów z komisj¹ A
    msgid_com = create_msg(msg_key_com);

    key_t msg_key_dean = generate_key('D'); // inicjalizacja kolejki komunikatów z dziekanem
    msgid_dean = create_msg(msg_key_dean);

    key_t sem_key = generate_key('G'); // klucz do semaforów
    semid = create_sem(sem_key, 2); // zestaw semaforów: 0 - komisja B, 1 - komisja A

    key_t shm_key = generate_key('H'); // inicjalizacja pamiêci wspó³dzielonej z komisj¹ A, studentami oraz dziekanem
    shmid = create_shm(shm_key, sizeof(Student) * MAX_STUDENTS); // tworzenie pamiêci wspó³dzielonej
    shm_ptr = attach_shm(shmid); // do³¹czanie pamiêci wspó³dzielonej

    sleep(1);

    cout << "[" << getpid() << "] Komisja B gotowa do pracy." << endl;

    // odbieranie studentów z pamiêci wspó³dzielonej
    Student* students = static_cast<Student*>(shm_ptr);

    // przeprowadzanie czêœci teoretycznej
    while (true)
    {
        usleep(250000);

        sem_wait(semid, 1); // oczekiwanie na powiadomienie od komisji A

        for (int i = 0; i < 3; ++i)
        {
            string student_msg = receive_msg(msgid_com, 1);
            if (student_msg == "END") goto end_exam;

            int index = 0;
            if (!student_msg.empty() && all_of(student_msg.begin(), student_msg.end(), ::isdigit)) // sprawdzenie poprawnoœci danych komunikatu
                index = stoi(student_msg); // student_msg zawiera index studenta w pamiêci wspó³dzielonej
            else
                handle_error("Nieprawidlowy komunikat");

            Student student = students[index]; // przypisanie studenta do odpowiadaj¹cego studenta z pamiêci wspó³dzielonej, ktory zdal praktyke - listy

            usleep(100000);

            cout << "Komisja B przyjmuje studenta o ID = " << student.id << endl;

            simulate_answers(student, 'B'); // symulujemy zadawanie pytañ i odpowiedzi dla studenta z listy

            // zapisanie wyników do pamiêci
            students[index].theoric_grade = student.theoric_grade;
            students[index].theoric_pass = student.theoric_pass;

            cout << "Komisja B ocenila studenta o ID = " << student.id << " za teorie: " << student.theoric_grade << endl << endl;

            string result_msg = to_string(index);
            send_msg(msgid_dean, 1, result_msg); // wysy³anie informacji o studencie koñcz¹cym egzamin do dziekana
        }

        sem_signal(semid, 0); // odblokowanie komisji A dla kolejnej grupy studentów
    }

    end_exam:
    send_msg(msgid_dean, 1, "END"); // powiadomienie dziekana o zakoñczeniu egzaminu
    cout << "[" << getpid() << "] Komisja B przeslala wyniki do dziekana." << endl;

    destroy_msg(msgid_com); // usuniêcie kolejki komunikatów z komisj¹ A
    destroy_sem(semid); // usuniêcie semaforów do obs³ugi komisji A
    detach_shm(shm_ptr); // od³aczenie wskaŸnika pamiêci wspó³dzielonej z komisj¹ A, studentami oraz dziekanem

	// cleanup(msgid_com, semid, shm_ptr, shmid_com); lub z pomoc¹ funkcji cleanup

	cout << "[" << getpid() << "] Komisja B konczy prace." << endl << endl;

	return 0;
}

void handle_signal(int signum)  // obs³uga czyszczenia zasobów dla komisji
{
    usleep(750000);
	send_msg(msgid_dean, 1, "END"); // awaryjne powiadomienie dziekana o zakoñczeniu egzaminowania
	cout << "Komisja B przerywa proces egzaminu." << endl;
	cleanup(msgid_com, semid, shm_ptr, -1); // uniwersalna funkcja czyszcz¹ca elementy ipc
	cout << "Wyczyszczono zasoby dla komisji B." << endl;
	exit(EXIT_SUCCESS);
}