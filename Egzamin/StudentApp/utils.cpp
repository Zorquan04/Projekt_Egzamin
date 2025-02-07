#include "utils.h"
#include "ipc.h"
#include "constants.h"

void handle_error(const string& message) // obs�uga b��d�w
{
    perror(message.c_str()); // wy�wietlenie zawarto�ci b��du
    raise(SIGINT); // wys�anie sygna�u przerwania
}

void send_signal(vector<pid_t>& child_pids, pid_t student, pid_t commissionA, pid_t commissionB) // przes�anie sygna�u o awarii oraz zako�czenie proces�w
{
    srand(static_cast<unsigned int>(time(NULL)));
    int random_time = rand() % 6 + 5; // mi�dzy 5 a 10s od wys�ania informacji o kierunku do student�w
    sleep(random_time);

    cout << red("DZIEKAN PRZERYWA EGZAMIN - ALARM!") << endl;
    kill(student, SIGUSR1);
    kill(commissionA, SIGUSR1);
    kill(commissionB, SIGUSR1);

    for (pid_t pids : child_pids)
        kill(pids, SIGUSR1);

    cout << blue("Komisja B przeslala wyniki do dziekana.") << endl;
}

void cleanup(int msg, int sem1, int sem2, void* ptr1, void* ptr2, int shm1, int shm2) // czyszczenie istniej�cych struktur (po przerwaniu awaryjnym)
{
    if (msg && msg != -1)
        destroy_msg(msg);
    if (sem1 && sem1 != -1)
        destroy_sem(sem1);
    if (sem2 && sem2 != -1)
        destroy_sem(sem2);
    if (ptr1 && ptr1 != NULL)
        detach_shm(ptr1);
    if (ptr2 && ptr2 != NULL)
        detach_shm(ptr2);
    if (shm1 && shm1 != -1)
        destroy_shm(shm1);
    if (shm2 && shm2 != -1)
        destroy_shm(shm2);
}

int get_direction() // pobranie kierunku pisz�cego egzamin przez u�ytkownika
{
    string input;
    while (true)
    {
        cout << purple("Wybierz numer kierunku, ktory podejdzie do egzaminu (1-5): ");
        getline(cin, input);

        try
        {
            size_t pos;
            int direction = stoi(input, &pos); // konwersja na int, `pos` wskazuje na pierwszy nieprzetworzony znak - obs�uga podania liczby z u�amkiem
            if (pos == input.size() && direction >= 1 && direction <= NUM_DIRECTIONS) // sprawdzamy zakres oraz czy pos zawiera identyczn� liczb� co podana przez u�ytkownika
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

// funkcje do kolorowych napis�w
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