#include "ipc.h"
#include "utils.h"
#include <sys/types.h>
#include <unistd.h>

// generowanie klucza IPC
key_t generate_key(const string& path, int project_id) 
{
    key_t key = ftok(path.c_str(), project_id); // generuje unikalny klucz IPC na podstawie œcie¿ki oraz ID
    if (key == -1) handle_error("Blad podczas generowania klucza IPC");
    return key;
}

// pamiêæ wspó³dzielona
int create_shm(key_t key, size_t size) 
{
    int shmid = shmget(key, size, IPC_CREAT | 0666); // tworzy segment pamiêci o podanym kluczu i rozmiarze
    if (shmid == -1) handle_error("Blad podczas tworzenia pamieci wspoldzielonej");
    return shmid;
}

void* attach_shm(int shmid) 
{
    void* addr = shmat(shmid, nullptr, 0); // do³¹cza segment pamiêci do przestrzeni adresowej procesu
    if (addr == (void*)-1) handle_error("Blad podczas dolaczania pamieci wspoldzielonej");
    return addr;
}

void detach_shm(void* shmaddr)
{
    if (shmdt(shmaddr) == -1) handle_error("Blad podczas odlaczania pamieci wspoldzielonej"); // od³¹cza segment pamiêci od przestrzeni adresowej procesu
}

void destroy_shm(int shmid) 
{
    if (shmctl(shmid, IPC_RMID, nullptr) == -1) handle_error("Blad podczas niszczenia pamieci wspoldzielonej"); // usuwa segment pamiêci z systemu
}

// semafory
int create_sem(key_t key, int semnum)
{
    int semid = semget(key, semnum, IPC_CREAT | 0666); // tworzy zestaw semaforów o podanym kluczu i liczbie semaforów
    if (semid == -1) handle_error("Blad podczas tworzenia semaforow");
    return semid;
}

void sem_wait(int semid, int semnum)
{
    struct sembuf operation = { static_cast<unsigned short>(semnum), -1, 0 }; // operacja P (dekrementacja) na semaforze (zmniejsza jego wartoœæ o 1, blokuj¹c, jeœli wartoœæ wynosi 0)
    if (semop(semid, &operation, 1) == -1) handle_error("Blad podczas operacji wait na semaforze");
    // jeœli wartoœæ semafora wynosi 0, proces zostaje zablokowany, a¿ inny proces zwolni semafor
}

void sem_signal(int semid, int semnum) 
{
    struct sembuf operation = { static_cast<unsigned short>(semnum), 1, 0 }; // operacja V (inkrementacja) na semaforze (zwiêksza jego wartoœæ o 1, odblokowuj¹c, jeœli procesy czekaj¹).
    if (semop(semid, &operation, 1) == -1) handle_error("Nie uda³o siê wykonaæ operacji signal na semaforze");
}

void destroy_sem(int semid) 
{
    if (semctl(semid, 0, IPC_RMID) == -1) handle_error("Blad podczas niszczenia semaforow"); // usuwa zestaw semaforów
}

// kolejka komunikatów
int create_msg(key_t key) 
{
    int msgid = msgget(key, IPC_CREAT | 0666); // tworzy kolejkê o podanym kluczu
    if (msgid == -1) handle_error("Blad podczas tworzenia kolejki komunikatow");
    return msgid;
}

void send_msg(int msgid, long mtype, const string& text) // wys³anie komunikatu o okreœlonym typie i treœci do kolejki
{
    struct Message 
    {
        long mtype; // typ komunikatu
        char mtext[256]; // treœæ komunikatu
    } msg = {}; // inicjalizacja struktury

    msg.mtype = mtype; // ustawienie typu
    strncpy(msg.mtext, text.c_str(), sizeof(msg.mtext) - 1); // kopiowanie treœci
    msg.mtext[sizeof(msg.mtext) - 1] = '\0'; // gwarancja zakoñczenia ³añcucha '\0'

    if (msgsnd(msgid, &msg, sizeof(msg.mtext), 0) == -1)
        handle_error("Blad podczas wysylania wiadomosci");
}

string receive_msg(int msgid, long mtype) // odebranie komunikatu o okreœlonym typie i treœci z kolejki
{
    struct Message 
    {
        long mtype;
        char mtext[256];
    } msg = {};

    if (msgrcv(msgid, &msg, sizeof(msg.mtext), mtype, 0) == -1)
        handle_error("Blad podczas odbierania wiadomosci");
    return string(msg.mtext); // zwrócenie treœci komunikatu jako string
}

void destroy_msg(int msgid) 
{
    if (msgctl(msgid, IPC_RMID, nullptr) == -1) // usuniêcie kolejki komunikatów z systemu
        handle_error("Blad podczas niszczenia kolejki komunikatow");
}
