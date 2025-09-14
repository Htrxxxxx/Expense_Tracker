// expense_tracker_oop.cpp
// Build: g++ -std=c++17 Code.cpp -o expense_tracker_oop
// Run:   ./expense_tracker_oop

#include <bits/stdc++.h>
using namespace std;


struct Expense {
    int id = 0;
    string date;     // YYYY-MM-DD
    string category;
    double amount = 0.0;
    string note;

    string serialize() const {
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
            try { e.id = stoi(parts[0]); } catch(...) { e.id = 0; }
            e.date = parts[1];
            e.category = parts[2];
            try { e.amount = stod(parts[3]); } catch(...) { e.amount = 0.0; }
            e.note = parts[4];
        }
        return e;
    }
};


class ExpenseStorage {
    string path_;
public:
    explicit ExpenseStorage(string path = "expenses.db") : path_(move(path)) {}

    vector<Expense> load() const {
        vector<Expense> out;
        ifstream ifs(path_);
        if (!ifs) return out;
        string line;
        while (getline(ifs, line)) {
            if (line.empty()) continue;
            Expense e = Expense::deserialize(line);
            if (e.id > 0) out.push_back(move(e));
        }
        return out;
    }

    bool save(const vector<Expense> &expenses) const {
        ofstream ofs(path_, ios::trunc);
        if (!ofs) return false;
        for (const auto &e : expenses) ofs << e.serialize() << '\n';
        return true;
    }
};


class ExpenseManager {
    vector<Expense> expenses_;
    int nextId_ = 1;
    ExpenseStorage storage_;

    void refreshNextId() {
        int maxId = 0;
        for (const auto &e : expenses_) if (e.id > maxId) maxId = e.id;
        nextId_ = maxId + 1;
    }

public:
    explicit ExpenseManager(const ExpenseStorage &storage = ExpenseStorage())
        : storage_(storage)
    {
        expenses_ = storage_.load();
        refreshNextId();
    }

    bool persist() const { return storage_.save(expenses_); }

    Expense addExpense(string date, string category, double amount, string note) {
        Expense e;
        e.id = nextId_++;
        e.date = move(date);
        e.category = move(category);
        e.amount = amount;
        e.note = move(note);
        expenses_.push_back(e);
        persist();
        return e;
    }

    bool removeExpense(int id) {
        size_t before = expenses_.size();
        expenses_.erase(remove_if(expenses_.begin(), expenses_.end(), [id](const Expense &x){ return x.id == id; }), expenses_.end());
        if (expenses_.size() < before) { persist(); return true; }
        return false;
    }

    bool editExpense(int id, string date, string category, double amount, string note) {
        for (auto &e : expenses_) {
            if (e.id == id) {
                e.date = move(date);
                e.category = move(category);
                e.amount = amount;
                e.note = move(note);
                persist();
                return true;
            }
        }
        return false;
    }

    vector<Expense> listAll() const { return expenses_; }

    vector<Expense> findByMonth(const string &yearMonth) const {
        vector<Expense> out;
        for (const auto &e : expenses_) if (e.date.size() >= 7 && e.date.substr(0,7) == yearMonth) out.push_back(e);
        return out;
    }

    unordered_map<string,double> totalPerCategory(const string &yearMonth) const {
        unordered_map<string,double> sums;
        for (const auto &e : expenses_) {
            if (e.date.size() >= 7 && e.date.substr(0,7) == yearMonth) sums[e.category] += e.amount;
        }
        return sums;
    }

    double totalForMonth(const string &yearMonth) const {
        double tot = 0.0;
        for (const auto &e : expenses_) if (e.date.size() >= 7 && e.date.substr(0,7) == yearMonth) tot += e.amount;
        return tot;
    }
};

class ExpenseCLI {
    ExpenseManager &mgr_;

    static void printExpense(const Expense &e) {
        cout << setw(3) << e.id << " | " << e.date << " | " << setw(10) << left << e.category << right
             << " | " << setw(8) << fixed << setprecision(2) << e.amount << " | " << e.note << '\n';
    }

    static void showMenu() {
        cout << "\n=== ExpenseTracker (OOP) ===\n";
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

    static string readLinePrompt(const string &prompt) {
        cout << prompt;
        string s;
        getline(cin, s);
        return s;
    }

    static bool readDouble(const string &prompt, double &out) {
        cout << prompt;
        string s;
        getline(cin, s);
        try {
            size_t idx = 0;
            out = stod(s, &idx);
            return idx > 0;
        } catch(...) { return false; }
    }

    static bool readInt(const string &prompt, int &out) {
        cout << prompt;
        string s; getline(cin, s);
        try {
            size_t idx = 0;
            out = stoi(s, &idx);
            return idx > 0;
        } catch(...) { return false; }
    }

public:
    explicit ExpenseCLI(ExpenseManager &mgr) : mgr_(mgr) {}

    void run() {
        bool running = true;
        while (running) {
            showMenu();
            string optStr; getline(cin, optStr);
            if (optStr.empty()) { cout << "Please choose an option.\n"; continue; }
            int opt = -1;
            try { opt = stoi(optStr); } catch(...) { cout << "Invalid selection.\n"; continue; }

            switch (opt) {
                case 1: {
                    auto all = mgr_.listAll();
                    if (all.empty()) cout << "No expenses recorded.\n";
                    else {
                        cout << "ID  | Date       | Category   |   Amount | Note\n";
                        cout << "----+------------+------------+----------+----------------\n";
                        for (const auto &e : all) printExpense(e);
                    }
                    break;
                }

                case 2: {
                    string date = readLinePrompt("Date (YYYY-MM-DD): ");
                    string category = readLinePrompt("Category: ");
                    double amount;
                    if (!readDouble("Amount: ", amount)) { cout << "Invalid amount.\n"; break; }
                    string note = readLinePrompt("Note (optional): ");
                    auto e = mgr_.addExpense(date, category, amount, note);
                    cout << "Added expense id=" << e.id << '\n';
                    break;
                }

                case 3: {
                    int id;
                    if (!readInt("ID to remove: ", id)) { cout << "Invalid id.\n"; break; }
                    if (mgr_.removeExpense(id)) cout << "Removed.\n"; else cout << "Not found.\n";
                    break;
                }

                case 4: {
                    int id;
                    if (!readInt("ID to edit: ", id)) { cout << "Invalid id.\n"; break; }
                    string date = readLinePrompt("New Date (YYYY-MM-DD): ");
                    string category = readLinePrompt("New Category: ");
                    double amount;
                    if (!readDouble("New Amount: ", amount)) { cout << "Invalid amount.\n"; break; }
                    string note = readLinePrompt("New Note: ");
                    if (mgr_.editExpense(id, date, category, amount, note)) cout << "Edited.\n"; else cout << "Not found.\n";
                    break;
                }

                case 5: {
                    string ym = readLinePrompt("Year-month (YYYY-MM): ");
                    auto v = mgr_.findByMonth(ym);
                    if (v.empty()) cout << "No expenses for " << ym << '\n';
                    else {
                        cout << "ID  | Date       | Category   |   Amount | Note\n";
                        cout << "----+------------+------------+----------+----------------\n";
                        for (const auto &e : v) printExpense(e);
                    }
                    break;
                }

                case 6: {
                    string ym = readLinePrompt("Year-month (YYYY-MM): ");
                    auto sums = mgr_.totalPerCategory(ym);
                    double total = mgr_.totalForMonth(ym);
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
                    if (mgr_.persist()) cout << "Saved.\n"; else cout << "Failed to save.\n";
                    break;

                case 8:
                    mgr_.persist();
                    cout << "Goodbye.\n";
                    running = false;
                    break;

                default:
                    cout << "Unknown option.\n";
            }
        }
    }
};

int main() {
    ExpenseStorage storage("expenses.db");
    ExpenseManager mgr(storage);
    ExpenseCLI cli(mgr);
    cli.run();
    return 0;
}
