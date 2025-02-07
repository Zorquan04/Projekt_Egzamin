#ifndef UTILS_H
#define UTILS_H

#include <iostream>
#include <csignal>
#include <cstring>
#include <string>
#include <cerrno>
#include <limits>
#include <vector>
#include <unistd.h>
#include <algorithm>
using namespace std;

void handle_error(const string&); // obs³uga b³êdów
void send_signal(vector<pid_t>&, pid_t, pid_t, pid_t); // wysy³anie alarmu przez dziekana
void cleanup(int, int, int, void*, void*, int, int); // sprz¹tanie zasobów
int get_direction(); // pobieranie kierunku od u¿ytkownika

// funkcje do kolorowych napisów
string green(const string&);
string red(const string&);
string blue(const string&);
string yellow(const string&);
string purple(const string&);

#endif