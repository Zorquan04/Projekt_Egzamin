#include "ipc.h"

// generowanie klucza IPC
key_t generate_key(const string& path, int project_id) 
{
    key_t key = ftok(path.c_str(), project_id); // generuje unikalny klucz IPC na podstawie �cie�ki oraz ID
    if (key == -1) handle_error("Blad podczas generowania klucza IPC");
    return key;
}

// pami�� wsp�dzielona
int create_shm(key_t key, size_t size) 
{
    int shmid = shmget(key, size, IPC_CREAT | 0666); // tworzy segment pami�ci o podanym kluczu i rozmiarze
    if (shmid == -1) handle_error("Blad podczas tworzenia pamieci wspoldzielonej");
    return shmid;
}

void* attach_shm(int shmid) 
{
    void* addr = shmat(shmid, nullptr, 0); // do��cza segment pami�ci do przestrzeni adresowej procesu
    if (addr == (void*)-1) handle_error("Blad podczas dolaczania pamieci wspoldzielonej");
    return addr;
}

void detach_shm(void* shmaddr)
{
    if (shmdt(shmaddr) == -1) handle_error("Blad podczas odlaczania pamieci wspoldzielonej"); // od��cza segment pami�ci od przestrzeni adresowej procesu
}

void destroy_shm(int shmid) 
{
    if (shmctl(shmid, IPC_RMID, nullptr) == -1) handle_error("Blad podczas niszczenia pamieci wspoldzielonej"); // usuwa segment pami�ci z systemu
}

// semafory
int create_sem(key_t key, int semnum)
{
    int semid = semget(key, semnum, IPC_CREAT | 0666); // tworzy zestaw semafor�w o podanym kluczu i liczbie semafor�w
    if (semid == -1) handle_error("Blad podczas tworzenia semaforow");
    
    return semid;
}

void sem_wait(int semid, int semnum)
{
    struct sembuf operation = { static_cast<unsigned short>(semnum), -1, 0 }; // operacja P (dekrementacja) na semaforze (zmniejsza jego warto�� o 1, blokuj�c, je�li warto�� wynosi 0)
    if (semop(semid, &operation, 1) == -1) handle_error("Blad podczas operacji wait na semaforze");
    // je�li warto�� semafora wynosi 0, proces zostaje zablokowany, a� inny proces zwolni semafor
}

void sem_signal(int semid, int semnum) 
{
    struct sembuf operation = { static_cast<unsigned short>(semnum), 1, 0 }; // operacja V (inkrementacja) na semaforze (zwi�ksza jego warto�� o 1, odblokowuj�c, je�li procesy czekaj�).
    if (semop(semid, &operation, 1) == -1) handle_error("Nie udalo sie wykonac operacji signal na semaforze");
}

void destroy_sem(int semid) 
{
    if (semctl(semid, 0, IPC_RMID) == -1) handle_error("Blad podczas niszczenia semaforow"); // usuwa zestaw semafor�w
}

// kolejka komunikat�w
int create_msg(key_t key) 
{
    int msgid = msgget(key, IPC_CREAT | 0666); // tworzy kolejk� o podanym kluczu
    if (msgid == -1) handle_error("Blad podczas tworzenia kolejki komunikatow");
    return msgid;
}

void send_msg(int msgid, long mtype, const string& text) // wys�anie komunikatu o okre�lonym typie i tre�ci do kolejki
{
    struct Message 
    {
        long mtype; // typ komunikatu
        char mtext[256]; // tre�� komunikatu
    } msg = {}; // inicjalizacja struktury

    msg.mtype = mtype; // ustawienie typu
    strncpy(msg.mtext, text.c_str(), sizeof(msg.mtext) - 1); // kopiowanie tre�ci
    msg.mtext[sizeof(msg.mtext) - 1] = '\0'; // gwarancja poprawnego zako�czenia �a�cucha

    if (msgsnd(msgid, &msg, sizeof(msg.mtext), 0) == -1)
        handle_error("Blad podczas wysylania wiadomosci");
}

string receive_msg(int msgid, long mtype) // odebranie komunikatu o okre�lonym typie i tre�ci z kolejki
{
    struct Message 
    {
        long mtype;
        char mtext[256];
    } msg = {};

    if (msgrcv(msgid, &msg, sizeof(msg.mtext), mtype, 0) == -1)
        handle_error("Blad podczas odbierania wiadomosci");

    return string(msg.mtext); // zwr�cenie tre�ci komunikatu jako string
}

void destroy_msg(int msgid) 
{
    if (msgctl(msgid, IPC_RMID, nullptr) == -1) // usuni�cie kolejki komunikat�w z systemu
        handle_error("Blad podczas niszczenia kolejki komunikatow");
}
