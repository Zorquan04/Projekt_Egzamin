# Kompilator i flagi
CXX = g++
CXXFLAGS = -std=c++17 -Wall -I$(PROJECT_DIR)/StudentApp -I$(PROJECT_DIR)/DeanApp -I$(PROJECT_DIR)/CommissionApp

# Lokacje folderów
PROJECT_DIR = .
STUDENT_DIR = $(PROJECT_DIR)/StudentApp
DEAN_DIR = $(PROJECT_DIR)/DeanApp
COMMISSION_DIR = $(PROJECT_DIR)/CommissionApp

# Pliki Ÿród³owe i nag³ówki
COMMON_SRCS = utils.cpp ipc.cpp exam_logic.cpp structures.cpp
COMMON_OBJS = $(COMMON_SRCS:.cpp=.o)

STUDENT_MAIN = StudentApp/main_student.cpp
DEAN_MAIN = DeanApp/main_dean.cpp
COMMISSION_MAIN = CommissionApp/main_commission.cpp

# Cele
all: student dean commission

student: $(addprefix $(STUDENT_DIR)/, $(COMMON_OBJS)) $(STUDENT_DIR)/$(STUDENT_MAIN)
	$(CXX) $(CXXFLAGS) -o $@ $^

dean: $(addprefix $(DEAN_DIR)/, $(COMMON_OBJS)) $(DEAN_DIR)/$(DEAN_MAIN)
	$(CXX) $(CXXFLAGS) -o $@ $^

commission: $(addprefix $(COMMISSION_DIR)/, $(COMMON_OBJS)) $(COMMISSION_DIR)/$(COMMISSION_MAIN)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Wspólna regu³a dla plików .o
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Czyszczenie
clean:
	rm -f $(STUDENT_DIR)/*.o $(DEAN_DIR)/*.o $(COMMISSION_DIR)/*.o student dean commission

.PHONY: all clean
