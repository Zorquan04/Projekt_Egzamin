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

void handle_error(const string&); // obs�uga b��d�w
void send_signal(vector<pid_t>&, pid_t, pid_t, pid_t); // wysy�anie alarmu przez dziekana
void cleanup(int, int, int, void*, void*, int, int); // sprz�tanie zasob�w
int get_direction(); // pobieranie kierunku od u�ytkownika

// funkcje do kolorowych napis�w
string green(const string&);
string red(const string&);
string blue(const string&);
string yellow(const string&);
string purple(const string&);

#endif