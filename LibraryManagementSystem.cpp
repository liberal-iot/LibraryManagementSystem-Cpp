
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <ctime>
using namespace std;

class Book {
public:
    string title, author, isbn, category;
    bool isBorrowed;

    Book() : isBorrowed(false) {}
    Book(string t, string a, string i, string c, bool b = false)
        : title(t), author(a), isbn(i), category(c), isBorrowed(b) {}

    void display() const {
        cout << "Title: " << title << ", Author: " << author
             << ", ISBN: " << isbn << ", Category: " << category
             << ", Status: " << (isBorrowed ? "Borrowed" : "Available") << endl;
    }

    string serialize() const {
        return title + "," + author + "," + isbn + "," + category + "," + (isBorrowed ? "1" : "0") + "\n";
    }

    static Book deserialize(const string& line) {
        vector<string> tokens;
        size_t start = 0, end = line.find(',');
        while (end != string::npos) {
            tokens.push_back(line.substr(start, end - start));
            start = end + 1;
            end = line.find(',', start);
        }
        tokens.push_back(line.substr(start));
        return Book(tokens[0], tokens[1], tokens[2], tokens[3], tokens[4] == "1");
    }
};

class User {
public:
    string username;
    string password;
    string role;  // "admin" or "member"

    User() {}
    User(string u, string p, string r) : username(u), password(p), role(r) {}

    string serialize() const {
        return username + "," + password + "," + role + "\n";
    }

    static User deserialize(const string& line) {
        size_t pos1 = line.find(',');
        size_t pos2 = line.rfind(',');
        string u = line.substr(0, pos1);
        string p = line.substr(pos1 + 1, pos2 - pos1 - 1);
        string r = line.substr(pos2 + 1);
        return User(u, p, r);
    }
};

class BorrowRecord {
public:
    string username, isbn;
    string borrowDate;
    string returnDate;

    BorrowRecord() {}
    BorrowRecord(string u, string i, string b, string r = "")
        : username(u), isbn(i), borrowDate(b), returnDate(r) {}

    string serialize() const {
        return username + "," + isbn + "," + borrowDate + "," + returnDate + "\n";
    }

    static BorrowRecord deserialize(const string& line) {
        size_t p1 = line.find(',');
        size_t p2 = line.find(',', p1 + 1);
        size_t p3 = line.find(',', p2 + 1);
        string u = line.substr(0, p1);
        string i = line.substr(p1 + 1, p2 - p1 - 1);
        string b = line.substr(p2 + 1, p3 - p2 - 1);
        string r = line.substr(p3 + 1);
        return BorrowRecord(u, i, b, r);
    }
};

string getCurrentDate() {
    time_t now = time(0);
    tm* ltm = localtime(&now);
    char buf[20];
    strftime(buf, sizeof(buf), "%Y-%m-%d", ltm);
    return string(buf);
}

class Library {
private:
    vector<Book> books;
    vector<User> users;
    vector<BorrowRecord> records;
    string currentUser;
    string booksFile = "books.txt";
    string usersFile = "users.txt";
    string recordsFile = "borrowed.txt";

    void loadBooks() {
        books.clear();
        ifstream infile(booksFile);
        string line;
        while (getline(infile, line)) {
            if (!line.empty()) books.push_back(Book::deserialize(line));
        }
    }

    void saveBooks() const {
        ofstream outfile(booksFile);
        for (const auto& b : books)
            outfile << b.serialize();
    }

    void loadUsers() {
        users.clear();
        ifstream infile(usersFile);
        string line;
        while (getline(infile, line)) {
            if (!line.empty()) users.push_back(User::deserialize(line));
        }
    }

    void saveUsers() const {
        ofstream outfile(usersFile);
        for (const auto& u : users)
            outfile << u.serialize();
    }

    void loadRecords() {
        records.clear();
        ifstream infile(recordsFile);
        string line;
        while (getline(infile, line)) {
            if (!line.empty()) records.push_back(BorrowRecord::deserialize(line));
        }
    }

    void saveRecords() const {
        ofstream outfile(recordsFile);
        for (const auto& r : records)
            outfile << r.serialize();
    }

    User* login(const string& uname, const string& pwd) {
        for (auto& user : users)
            if (user.username == uname && user.password == pwd)
                return &user;
        return nullptr;
    }

    bool signup() {
        string uname, pwd;
        cout << "Choose a username: ";
        cin >> uname;
        cout << "Choose a password: ";
        cin >> pwd;

        // Check if username exists
        for (auto& u : users) {
            if (u.username == uname) {
                cout << "Username already taken.\n";
                return false;
            }
        }

        users.push_back(User(uname, pwd, "member"));
        saveUsers();
        cout << "Signup successful! Please login now.\n";
        return true;
    }

public:
    Library() {
        loadBooks();
        loadUsers();
        loadRecords();
    }

    void registerDefaultAdmin() {
        if (users.empty()) {
            users.push_back(User("admin", "admin123", "admin"));
            users.push_back(User("member", "member123", "member")); // demo member
            saveUsers();
        }
    }

    void run() {
        int initialChoice;
        do {
            cout << "\n===== Library Management System =====\n";
            cout << "1. Login\n2. Signup\n3. Exit\nChoose an option: ";
            cin >> initialChoice;
            cin.ignore();

            if (initialChoice == 2) {
                signup();
            } else if (initialChoice == 1) {
                string uname, pwd;
                cout << "Username: ";
                getline(cin, uname);
                cout << "Password: ";
                getline(cin, pwd);

                User* user = login(uname, pwd);
                if (!user) {
                    cout << "Invalid credentials.\n";
                    continue;
                }

                currentUser = user->username;
                cout << "Welcome, " << currentUser << " (" << user->role << ")\n";

                int choice;
                do {
                    if (user->role == "admin") {
                        cout << "\nAdmin Menu:\n";
                        cout << "1. Add Book\n2. View Books\n3. Delete Book\n4. View Borrow Records\n5. Logout\nEnter choice: ";
                        cin >> choice;
                        cin.ignore();
                        if (choice == 1) {
                            string t, a, i, c;
                            cout << "Title: ";
                            getline(cin, t);
                            cout << "Author: ";
                            getline(cin, a);
                            cout << "ISBN: ";
                            getline(cin, i);
                            cout << "Category: ";
                            getline(cin, c);
                            books.push_back(Book(t, a, i, c));
                            saveBooks();
                            cout << "Book added.\n";
                        } else if (choice == 2) {
                            for (const auto& b : books)
                                b.display();
                        } else if (choice == 3) {
                            string i;
                            cout << "Enter ISBN to delete: ";
                            getline(cin, i);
                            size_t before = books.size();
                            books.erase(remove_if(books.begin(), books.end(), [&](const Book& b) { return b.isbn == i; }), books.end());
                            if (books.size() < before) {
                                saveBooks();
                                cout << "Book deleted.\n";
                            } else {
                                cout << "Book not found.\n";
                            }
                        } else if (choice == 4) {
                            for (const auto& r : records) {
                                cout << "User: " << r.username << ", ISBN: " << r.isbn
                                     << ", Borrowed: " << r.borrowDate
                                     << ", Returned: " << (r.returnDate.empty() ? "Not returned" : r.returnDate) << endl;
                            }
                        }
                    } else { // member
                        cout << "\nMember Menu:\n";
                        cout << "1. Search Book\n2. Borrow Book\n3. Return Book\n4. View My Borrow History\n5. Logout\nEnter choice: ";
                        cin >> choice;
                        cin.ignore();
                        if (choice == 1) {
                            string key;
                            cout << "Enter ISBN or Title: ";
                            getline(cin, key);
                            bool found = false;
                            for (const auto& b : books) {
                                if (b.title == key || b.isbn == key) {
                                    b.display();
                                    found = true;
                                }
                            }
                            if (!found) cout << "No book found.\n";
                        } else if (choice == 2) {
                            string i;
                            cout << "Enter ISBN to borrow: ";
                            getline(cin, i);
                            bool borrowed = false;
                            for (auto& b : books) {
                                if (b.isbn == i) {
                                    if (!b.isBorrowed) {
                                        b.isBorrowed = true;
                                        records.push_back(BorrowRecord(currentUser, i, getCurrentDate()));
                                        saveBooks();
                                        saveRecords();
                                        cout << "Book borrowed.\n";
                                        borrowed = true;
                                    } else {
                                        cout << "Book is already borrowed.\n";
                                    }
                                    break;
                                }
                            }
                            if (!borrowed) cout << "Book not found.\n";
                        } else if (choice == 3) {
                            string i;
                            cout << "Enter ISBN to return: ";
                            getline(cin, i);
                            bool returned = false;
                            for (auto& b : books) {
                                if (b.isbn == i && b.isBorrowed) {
                                    b.isBorrowed = false;
                                    for (auto& r : records) {
                                        if (r.username == currentUser && r.isbn == i && r.returnDate.empty()) {
                                            r.returnDate = getCurrentDate();
                                            break;
                                        }
                                    }
                                    saveBooks();
                                    saveRecords();
                                    cout << "Book returned.\n";
                                    returned = true;
                                    break;
                                }
                            }
                            if (!returned) cout << "Book not found or not borrowed.\n";
                        } else if (choice == 4) {
                            bool found = false;
                            for (const auto& r : records) {
                                if (r.username == currentUser) {
                                    cout << "ISBN: " << r.isbn << ", Borrowed: " << r.borrowDate
                                         << ", Returned: " << (r.returnDate.empty() ? "Not returned" : r.returnDate) << endl;
                                    found = true;
                                }
                            }
                            if (!found) cout << "No borrow history found.\n";
                        }
                    }
                } while (choice != 5);
            }
        } while (initialChoice != 3);

        cout << "Goodbye!\n";
    }
};

int main() {
    Library lib;
    lib.registerDefaultAdmin();
    lib.run();
    return 0;
}
