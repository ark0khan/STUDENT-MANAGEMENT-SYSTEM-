#include <iostream>
#include <vector>
#include <algorithm>
using namespace std;

class Student {
public:
    int id;
    string name;
    double gpa;
    Student(int i, string n, double g) : id(i), name(n), gpa(g) {}
};

class University {
public:
    string name;
    vector<Student> students;
    University(string n) : name(n) {}
    void addStudent(int id, string name, double gpa) {
        students.push_back(Student(id, name, gpa));
    }
    void display() {
        cout << "\nUniversity: " << name << endl;
        if (students.empty()) {
            cout << "No students enrolled.\n";
            return;
        }
        for (auto &s : students) {
            cout << "ID: " << s.id << " | Name: " << s.name << " | GPA: " << s.gpa << endl;
        }
    }
    void sortByGPA() {
        sort(students.begin(), students.end(), [](Student &a, Student &b) {
            return a.gpa > b.gpa;
        });
        cout << "Students sorted by GPA.\n";
    }
    void sortByName() {
        sort(students.begin(), students.end(), [](Student &a, Student &b) {
            return a.name < b.name;
        });
        cout << "Students sorted by Name.\n";
    }
    void searchByID(int id) {
        auto it = find_if(students.begin(), students.end(), [id](Student &s) {
            return s.id == id;
        });
        if (it != students.end()) {
            cout << "Found -> ID: " << it->id << " | Name: " << it->name << " | GPA: " << it->gpa << endl;
        } else {
            cout << "Student not found.\n";
        }
    }
    void searchByName(string name) {
        bool found = false;
        for (auto &s : students) {
            if (s.name == name) {
                cout << "Found -> ID: " << s.id << " | Name: " << s.name << " | GPA: " << s.gpa << endl;
                found = true;
            }
        }
        if (!found) cout << "Student not found.\n";
    }
    void removeStudent(int id) {
        auto it = remove_if(students.begin(), students.end(), [id](Student &s) {
            return s.id == id;
        });
        if (it != students.end()) {
            students.erase(it, students.end());
            cout << "Student removed.\n";
        } else {
            cout << "Student not found.\n";
        }
    }
};

int main() {
    University ewU("East West University");
    int choice;
    do {
        cout << "\n--- " << ewU.name << " Management System ---\n";
        cout << "1. Add Student\n";
        cout << "2. Display All Students\n";
        cout << "3. Sort Students by GPA\n";
        cout << "4. Sort Students by Name\n";
        cout << "5. Search Student by ID\n";
        cout << "6. Search Student by Name\n";
        cout << "7. Remove Student by ID\n";
        cout << "0. Exit\n";
        cout << "Enter choice: ";
        cin >> choice;
        if (choice == 1) {
            int id; string name; double gpa;
            cout << "Enter ID: "; cin >> id;
            cout << "Enter Name: "; cin >> name;
            cout << "Enter GPA: "; cin >> gpa;
            ewU.addStudent(id, name, gpa);
        } else if (choice == 2) {
            ewU.display();
        } else if (choice == 3) {
            ewU.sortByGPA();
        } else if (choice == 4) {
            ewU.sortByName();
        } else if (choice == 5) {
            int id;
            cout << "Enter ID: "; cin >> id;
            ewU.searchByID(id);
        } else if (choice == 6) {
            string name;
            cout << "Enter Name: "; cin >> name;
            ewU.searchByName(name);
        } else if (choice == 7) {
            int id;
            cout << "Enter ID: "; cin >> id;
            ewU.removeStudent(id);
        }
    } while (choice != 0);
    return 0;
}

