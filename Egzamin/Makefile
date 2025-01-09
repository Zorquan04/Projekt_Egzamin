# Kompilator i flagi
CXX = g++
CXXFLAGS = -std=c++17 -Wall -I/home/zorquan/projects/StudentApp -I/home/zorquan/projects/DeanApp -I/home/zorquan/projects/CommissionApp

# Lokacje folderów
PROJECT_DIR = /home/zorquan/projects
STUDENT_DIR = $(PROJECT_DIR)/StudentApp
DEAN_DIR = $(PROJECT_DIR)/DeanApp
COMMISSION_DIR = $(PROJECT_DIR)/CommissionApp

# Œcie¿ki do plików g³ównych
STUDENT_MAIN = $(STUDENT_DIR)/StudentApp/main_student.cpp
DEAN_MAIN = $(DEAN_DIR)/DeanApp/main_dean.cpp
COMMISSION_MAIN = $(COMMISSION_DIR)/CommissionApp/main_commission.cpp

# Wspólne pliki Ÿród³owe i obiekty
COMMON_SRCS = utils.cpp ipc.cpp exam_logic.cpp
STUDENT_OBJS = $(addprefix $(STUDENT_DIR)/, $(COMMON_SRCS:.cpp=.o))
DEAN_OBJS = $(addprefix $(DEAN_DIR)/, $(COMMON_SRCS:.cpp=.o))
COMMISSION_OBJS = $(addprefix $(COMMISSION_DIR)/, $(COMMON_SRCS:.cpp=.o))

# Cele
all: student_app dean_app commission_app

student_app: $(STUDENT_OBJS) $(STUDENT_MAIN)
	$(CXX) $(CXXFLAGS) -o $@ $(STUDENT_MAIN) $(STUDENT_OBJS)

dean_app: $(DEAN_OBJS) $(DEAN_MAIN)
	$(CXX) $(CXXFLAGS) -o $@ $(DEAN_MAIN) $(DEAN_OBJS)

commission_app: $(COMMISSION_OBJS) $(COMMISSION_MAIN)
	$(CXX) $(CXXFLAGS) -o $@ $(COMMISSION_MAIN) $(COMMISSION_OBJS)

# Regu³a dla wspólnych plików .o
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(STUDENT_DIR)/*.o $(DEAN_DIR)/*.o $(COMMISSION_DIR)/*.o student_app dean_app commission_app
