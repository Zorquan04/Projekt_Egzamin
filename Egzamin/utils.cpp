#include "utils.h"
#include "ipc.h"
#include "constants.h"

void handle_error(const string& message) // obs³uga b³êdów
{
    perror(message.c_str()); // wyœwietlenie zawartoœci b³êdu
    raise(SIGINT); // wys³anie sygna³u przerwania
}

void send_signal(pid_t student, pid_t commissionA, pid_t commissionB) // przes³anie sygna³u o awarii oraz zakoñczenie procesów
{
    srand(static_cast<unsigned int>(time(NULL)));
    int random_time = rand() % 11 + 10; // miêdzy 10 a 20s od wys³ania informacji o kierunku do studentów
    sleep(random_time);

    cout << red("DZIEKAN PRZERYWA EGZAMIN - ALARM!") << endl;
    kill(student, SIGINT);
    kill(commissionA, SIGINT);
    kill(commissionB, SIGINT);
    cout << blue("Komisja B przeslala wyniki do dziekana.") << endl;
}

void cleanup(int msg, int sem, void* ptr, int shm) // czyszczenie istniej¹cych struktur (po przerwaniu awaryjnym)
{
    if (msg && msg != -1)
        destroy_msg(msg);
    if (sem && sem != -1)
        destroy_sem(sem);
    if (ptr && ptr != NULL)
        detach_shm(ptr);
    if (shm && shm != -1)
        destroy_shm(shm);
}

int get_direction() // pobranie kierunku pisz¹cego egzamin przez u¿ytkownika
{
    string input;
    while (true)
    {
        cout << purple("Wybierz numer kierunku, ktory podejdzie do egzaminu (1-5): ");
        getline(cin, input);

        try
        {
            size_t pos;
            int direction = stoi(input, &pos); // konwersja na int, `pos` wskazuje na pierwszy nieprzetworzony znak - obs³uga podania liczby z u³amkiem
            if (pos == input.size() && direction >= 1 && direction <= 5) // sprawdzamy zakres oraz czy pos zawiera identyczn¹ liczbê co podana przez u¿ytkownika
                return direction;
            else
                cout << red("Numer poza zakresem lub liczba z ulamkiem. Sprobuj ponownie.") << endl;
        }
        catch (...)
        {
            cout << red("Nieprawidlowy typ danych. Sprobuj ponownie.") << endl;
        }
    }
}

// funkcje do kolorowych napisów
string green(const string& msg)
{
    return "\033[92m" + msg + "\033[0m";
}

string red(const string& msg)
{
    return "\033[91m" + msg + "\033[0m";
}

string blue(const string& msg)
{
    return "\033[94m" + msg + "\033[0m";
}

string yellow(const string& msg)
{
    return "\033[93m" + msg + "\033[0m";
}

string purple(const string& msg)
{
    return "\033[95m" + msg + "\033[0m";
}