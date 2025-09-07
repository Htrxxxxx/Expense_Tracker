// expense_tracker.cpp
// Build: g++ -std=c++17 expense_tracker.cpp -o expense_tracker
// Run:   ./expense_tracker
//
// Simple single-file Expense Tracker (no external libs).
// Data stored in "expenses.db" (pipe-separated fields per line).

#include <bits/stdc++.h>
using namespace std;

struct Expense {
    int id = 0;
    string date;     // YYYY-MM-DD
    string category;
    double amount = 0.0;
    string note;

    string serialize() const {
        // Replace any '|' or newline with space to keep format sane.
        auto clean = [](string s){
            for (char &c : s) if (c == '|' || c == '\n' || c == '\r') c = ' ';
            return s;
        };
        ostringstream oss;
        oss << id << '|' << date << '|' << category << '|' << fixed << setprecision(2) << amount << '|' << clean(note);
        return oss.str();
    }

    static Expense deserialize(const string &line) {
        Expense e;
        vector<string> parts;
        string cur;
        for (char c : line) {
            if (c == '|') { parts.push_back(cur); cur.clear(); }
            else cur.push_back(c);
        }
        parts.push_back(cur);
        if (parts.size() >= 5) {
            try {
                e.id = stoi(parts[0]);
            } catch(...) { e.id = 0; }
            e.date = parts[1];
            e.category = parts[2];
            try {
                e.amount = stod(parts[3]);
            } catch(...) { e.amount = 0.0; }
            e.note = parts[4];
        }
        return e;
    }
};

class ExpenseManager {
    vector<Expense> expenses;
    int nextId = 1;
    string storagePath;

public:
    ExpenseManager(const string &path = "expenses.db") : storagePath(path) { load(); }

    bool load() {
        expenses.clear();
        ifstream ifs(storagePath);
        if (!ifs) { nextId = 1; return false; }
        string line;
        int maxId = 0;
        while (getline(ifs, line)) {
            if (line.empty()) continue;
            Expense e = Expense::deserialize(line);
            if (e.id > 0) {
                expenses.push_back(e);
                if (e.id > maxId) maxId = e.id;
            }
        }
        nextId = maxId + 1;
        return true;
    }

    bool save() const {
        ofstream ofs(storagePath, ios::trunc);
        if (!ofs) return false;
        for (const auto &e : expenses) ofs << e.serialize() << '\n';
        return true;
    }

    Expense addExpense(const string &date, const string &category, double amount, const string &note) {
        Expense e;
        e.id = nextId++;
        e.date = date;
        e.category = category;
        e.amount = amount;
        e.note = note;
        expenses.push_back(e);
        save();
        return e;
    }

    bool removeExpense(int id) {
        size_t before = expenses.size();
        expenses.erase(remove_if(expenses.begin(), expenses.end(), [id](const Expense &x){ return x.id == id; }), expenses.end());
        if (expenses.size() < before) { save(); return true; }
        return false;
    }

    bool editExpense(int id, const string &date, const string &category, double amount, const string &note) {
        for (auto &e : expenses) {
            if (e.id == id) {
                e.date = date;
                e.category = category;
                e.amount = amount;
                e.note = note;
                save();
                return true;
            }
        }
        return false;
    }

    vector<Expense> listAll() const { return expenses; }

    vector<Expense> findByMonth(const string &yearMonth) const {
        // yearMonth: "YYYY-MM"
        vector<Expense> out;
        for (const auto &e : expenses) if (e.date.size() >= 7 && e.date.substr(0,7) == yearMonth) out.push_back(e);
        return out;
    }

    unordered_map<string,double> totalPerCategory(const string &yearMonth) const {
        unordered_map<string,double> sums;
        for (const auto &e : expenses) {
            if (e.date.size() >= 7 && e.date.substr(0,7) == yearMonth) sums[e.category] += e.amount;
        }
        return sums;
    }

    double totalForMonth(const string &yearMonth) const {
        double tot = 0.0;
        for (const auto &e : expenses) if (e.date.size() >= 7 && e.date.substr(0,7) == yearMonth) tot += e.amount;
        return tot;
    }
};

// ---------- UI helpers ----------
void printExpense(const Expense &e) {
    cout << setw(3) << e.id << " | " << e.date << " | " << setw(10) << left << e.category << right
         << " | " << setw(8) << fixed << setprecision(2) << e.amount << " | " << e.note << '\n';
}

void showMenu() {
    cout << "\n=== ExpenseTracker ===\n";
    cout << "1. List all expenses\n";
    cout << "2. Add expense\n";
    cout << "3. Remove expense\n";
    cout << "4. Edit expense\n";
    cout << "5. List by month (YYYY-MM)\n";
    cout << "6. Report: totals per category for month\n";
    cout << "7. Save\n";
    cout << "8. Exit\n";
    cout << "Choose: ";
}

int main() {
    ExpenseManager mgr("expenses.db");
    bool running = true;
    while (running) {
        showMenu();
        int opt;
        if (!(cin >> opt)) {
            cin.clear();
            string junk; getline(cin, junk);
            cout << "Invalid selection.\n";
            continue;
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        switch (opt) {
            case 1: {
                auto all = mgr.listAll();
                if (all.empty()) cout << "No expenses recorded.\n";
                else {
                    cout << "ID  | Date       | Category   |   Amount | Note\n";
                    cout << "----+------------+------------+----------+----------------\n";
                    for (const auto &e : all) printExpense(e);
                }
                break;
            }

            case 2: {
                string date, category, note;
                double amount;
                cout << "Date (YYYY-MM-DD): "; getline(cin, date);
                cout << "Category: "; getline(cin, category);
                cout << "Amount: "; if (!(cin >> amount)) { cin.clear(); string tmp; getline(cin, tmp); cout << "Invalid amount.\n"; break; }
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                cout << "Note (optional): "; getline(cin, note);
                auto e = mgr.addExpense(date, category, amount, note);
                cout << "Added expense id=" << e.id << '\n';
                break;
            }

            case 3: {
                cout << "ID to remove: "; int id; if (!(cin >> id)) { cin.clear(); string tmp; getline(cin,tmp); cout << "Invalid id.\n"; break; }
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                if (mgr.removeExpense(id)) cout << "Removed.\n"; else cout << "Not found.\n";
                break;
            }

            case 4: {
                cout << "ID to edit: "; int id; if (!(cin >> id)) { cin.clear(); string tmp; getline(cin,tmp); cout << "Invalid id.\n"; break; }
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                string date, category, note;
                double amount;
                cout << "New Date (YYYY-MM-DD): "; getline(cin, date);
                cout << "New Category: "; getline(cin, category);
                cout << "New Amount: "; if (!(cin >> amount)) { cin.clear(); string tmp; getline(cin,tmp); cout << "Invalid amount.\n"; break; }
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                cout << "New Note: "; getline(cin, note);
                if (mgr.editExpense(id, date, category, amount, note)) cout << "Edited.\n"; else cout << "Not found.\n";
                break;
            }

            case 5: {
                cout << "Year-month (YYYY-MM): "; string ym; getline(cin, ym);
                auto v = mgr.findByMonth(ym);
                if (v.empty()) cout << "No expenses for " << ym << '\n';
                else {
                    cout << "ID  | Date       | Category   |   Amount | Note\n";
                    cout << "----+------------+------------+----------+----------------\n";
                    for (const auto &e : v) printExpense(e);
                }
                break;
            }

            case 6: {
                cout << "Year-month (YYYY-MM): "; string ym; getline(cin, ym);
                auto sums = mgr.totalPerCategory(ym);
                double total = mgr.totalForMonth(ym);
                if (sums.empty()) cout << "No data for " << ym << '\n';
                else {
                    cout << "Totals for " << ym << ":\n";
                    for (auto &p : sums) cout << setw(12) << left << p.first << " -> " << fixed << setprecision(2) << right << p.second << '\n';
                    cout << "--------------------\n";
                    cout << "Total -> " << fixed << setprecision(2) << total << '\n';
                }
                break;
            }

            case 7:
                if (mgr.save()) cout << "Saved.\n"; else cout << "Failed to save.\n";
                break;

            case 8:
                mgr.save();
                cout << "Goodbye.\n";
                running = false;
                break;

            default:
                cout << "Unknown option.\n";
        }
    }
    return 0;
}
