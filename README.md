## ExpenseTracker : 
- Minimal single-file C++17 CLI to record and report personal expenses. Saves data in a simple pipe-separated file expenses.db — no external libraries.

## Features :
- Add / edit / remove expenses (id, date, category, amount, note)
- List all or by month (YYYY-MM)
- Monthly totals per category
- Auto-saves to expenses.db
- Uses simple serialization / deserialization (convert Expense ↔ line) for persistent storage

## Build & Run : 
g++ -std=c++17 expense_tracker.cpp -o expense_tracker
./expense_tracker        # (Windows: expense_tracker.exe)

## Storage : 
Each line in expenses.db:
id|date|category|amount|note

### Example
<details>
<summary>Full example session (click to expand)</summary>

```bash
# Start program
$ ./expense_tracker
=== ExpenseTracker ===
1. List all expenses
2. Add expense
3. Remove expense
4. Edit expense
5. List by month (YYYY-MM)
6. Report: totals per category for month
7. Save
8. Exit
Choose: 2

# Add an expense
Date (YYYY-MM-DD): 2025-09-10
Category: Coffee
Amount: 3.50
Note: Morning
Added expense id=1

# List all expenses
Choose: 1
ID  | Date       | Category   |   Amount | Note
----+------------+------------+----------+----------------
  1 | 2025-09-10 | Coffee     |     3.50 | Morning

# Monthly report
Choose: 6
Year-month (YYYY-MM): 2025-09
Totals for 2025-09:
Coffee       -> 3.50
--------------------
Total -> 3.50

# Exit (auto-saves)
Choose: 8
Goodbye.
