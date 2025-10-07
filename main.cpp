#include <iomanip>
#include <iostream>
#include <fstream>
#include <sstream>
#include <conio.h>
#include <algorithm>
#include <stdexcept>
#include <chrono>
#include <windows.h>
#include <dirent.h>  
#define BANKNAME "PHILBANK"
#define FILEPATH "bankDataBase.txt"

using namespace std;

void setColor(int r, int g, int b) {
    cout << "\033[38;2;" << r << ";" << g << ";" << b << "m";
}

void resetColor() {
    cout << "\033[0m";
}
// Bright green
void setGreen() {
    setColor(0, 255, 0); 
}

// Bright yellow
void setYellow() {
    setColor(255, 255, 0); 
}

struct Account{
    string accountNo;
    string pinCode;
    double balance;
    string accType;
    Account* next;
};

struct CustomersData{
    string accountName;
    string birthday;
    string contactNo;
    Account* newAccount;
};

struct DataBase{
    CustomersData infos;
    Account account;
    DataBase* next;
    DataBase* prev;

    DataBase(CustomersData data):infos(data), prev(NULL), next(NULL) {}
};

class Bank{
    private: 
        DataBase* head;
        DataBase* tail;
        // Track logged in account
        string currentLoggedInAccount; 
    public:
        Bank() : head(NULL), tail(NULL), currentLoggedInAccount("") {}

        DataBase* getHead(){ return head;}
        DataBase* getTail(){ return tail;}
        void setCurrentAccount(const string& accNo) { currentLoggedInAccount = accNo; }
        string getCurrentAccount() { return currentLoggedInAccount; }

        void balanceInquiry(string accNo);
        void withdraw(double getAmount, DataBase* cur);
        void deposit(double depositAmount, DataBase* cur);
        void changePin(DataBase* account, const string& oldPin, const string& newPin, const string& confirmPin);
        void fundTransfer(double amountToTransfer, DataBase* sender, string recieverNo);
        void viewAccounts();
        void addToDatabase(DataBase* newNode);
        DataBase* findAccount(const string& recieverNo);
        void save(const string& filename);
        void load(const string& filename);
};

class Security{
    private: int v = 120;
    public:
        string getPinCode();
        string encrypt(string pin);
        string decrypt(string pin);
        void savePin(const string& pin, const string& DRIVE, const string& accNo);
        bool verifyPin(Account* acc, const string& inputPin, const string& DRIVE);
};

class NewAccount : public Bank, public Security{
    public:
    void openAccount(Bank& bank, const string& DRIVE);
    void createPin(CustomersData& d, const string DRIVE, string pin);
    void createAccNo(CustomersData& d, const string& pin, double initDeposit, const string& accounType);
    void initialDeposit(double depositAmount, DataBase* cur);
    void addAccount(CustomersData &customer, const string& accNo, const string& pin, double initDeposit, const string& accountType);
};

int bankInterface();
bool isValidPinFormat(const string& pin);
void waitForCard(const string& drive);
void transactionDisplay(int i);

void firstTimeUsers(const string& DRIVE, Bank& bank);
bool pinFileExists(const string& drive);

int main() {

    Bank bank;
    bank.load(FILEPATH);
    NewAccount openAcc;
    Security sec;
    string DRIVE = "D:"; 
    waitForCard(DRIVE);
    system("cls");

    firstTimeUsers(DRIVE, bank);

    int attempts = 3;
    bool accessGranted = false;
    string loggedInAccountNo = "";

    while (attempts > 0 && !accessGranted) {
        string enteredPin = sec.getPinCode();

        DataBase* acc = bank.getHead();
        while (acc != nullptr) {
            Account* account = acc->infos.newAccount;
            while (account != nullptr) {
                if (sec.verifyPin(account, enteredPin, DRIVE)) {
                    accessGranted = true;
                    loggedInAccountNo = account->accountNo;
                    bank.setCurrentAccount(loggedInAccountNo);
                    break;
                }
                account = account->next;
            }
            if (accessGranted) break;
            acc = acc->next;
        }

        if (!accessGranted) {
            attempts--;
            if (attempts > 0) {
                setGreen();
                cout << "Incorrect PIN. You have " << attempts << " attempt(s) left.\n";
                resetColor();
            } else {
                setGreen();
                cout << "Access Denied. No attempts left.\n";
                resetColor();
                system("pause");
                return 1;
            }
        }
    }

    Sleep(1500);
    system("cls");
    
    int choice;
    do {
        system("cls");
        
        switch (bankInterface()) {
        case 1: {
            system("cls");
            openAcc.openAccount(bank, DRIVE);
            bank.save(FILEPATH);
            system("pause");
            break;
        }
        case 2: {
            system("cls");
            transactionDisplay(2);
            string accNo;
            double amount;
            setGreen();
            cout << "Enter Account Number: ";
            resetColor();
            cin >> accNo;

            DataBase* cur = bank.findAccount(accNo);
            if (cur) {
                setGreen();
                cout << "Enter deposit amount: ";
                resetColor();
                cin >> amount;
                bank.deposit(amount, cur);
                bank.save(FILEPATH);
            } else {
                setGreen();
                cout << "Account not found.\n";
                resetColor();
            }
            system("pause");
            break;
        }
        case 3: {
            system("cls");
            transactionDisplay(1);
            string accNo;
            double amount;
            setGreen();
            cout << "Enter Account Number: ";
            resetColor();
            cin >> accNo;
            DataBase* cur = bank.findAccount(accNo);
            if (cur) {
                setGreen();
                cout << "Enter amount to withdraw: ";
                resetColor();
                cin >> amount;
                bank.withdraw(amount, cur);
                bank.save(FILEPATH);
            } else {
                setGreen();
                cout << "Account not found.\n";
                resetColor();
            }
            system("pause");
            break;
        }
        case 4: {
            system("cls");
            transactionDisplay(3);
            string senderNo, receiverNo;
            double amount;
            setGreen();
            cout << "Enter your Account Number: ";
            resetColor();
            cin >> senderNo;

            DataBase* sender = bank.findAccount(senderNo);
            if (sender) {
                setGreen();
                cout << "Enter receiver's Account Number: ";
                resetColor();
                cin >> receiverNo;
                setGreen();
                cout << "Enter amount to transfer: ";
                resetColor();
                cin >> amount;
                bank.fundTransfer(amount, sender, receiverNo);
                bank.save(FILEPATH);
            } else {
                setGreen();
                cout << "Sender account not found.\n";
                resetColor();
            }
            system("pause");
            break;
        }
        case 5: {
            system("cls");
            transactionDisplay(4);
            string accNo, oldPin, newPin, confirmPin;
            setGreen();
            cout << "Enter Account Number: ";
            resetColor();
            cin >> accNo;

            DataBase* acc = bank.findAccount(accNo);
            if (acc) {
                setGreen();
                cout << "Enter old PIN: ";
                resetColor();
                oldPin = sec.getPinCode();
                setGreen();
                cout << "Enter new PIN: ";
                resetColor();
                newPin = sec.getPinCode();
                setGreen();
                cout << "Confirm new PIN: ";
                resetColor();
                confirmPin = sec.getPinCode();
                
                Security encryptor;
                string encryptedOld = encryptor.encrypt(oldPin);
                bank.changePin(acc, encryptedOld, newPin, confirmPin);
                
                if(newPin == confirmPin && isValidPinFormat(newPin)){
                    string encryptedNew = encryptor.encrypt(newPin);
                    encryptor.savePin(encryptedNew, DRIVE, accNo);
                    bank.save(FILEPATH);
                }
            } else {
                setGreen();
                cout << "Account not found.\n";
                resetColor();
            }
            system("pause");
            break;
        }
        case 6: {
            system("cls");
            transactionDisplay(5);
            bank.viewAccounts();
            system("pause");
            break;
        }
        case 7: {
            system("cls");
            string accNo;
            transactionDisplay(0);
            setGreen();
            cout << "Enter Account Number: ";
            resetColor();
            cin >> accNo;
            bank.balanceInquiry(accNo);
            system("pause");
            break;
        }
        case 8:
            bank.save(FILEPATH);
            setGreen();
            cout << "Exiting program... Thank you for using " << BANKNAME << "!\n";
            resetColor();
            exit(0);
        default:
            setGreen();
            cout << "Invalid choice. Try again.\n";
            resetColor();
            system("pause");
        }

    } while (choice != 8);

    return 0;
}


bool pinFileExists(const string& drive) {
    string folderPath = drive + "/Private";
    DIR* dir = opendir(folderPath.c_str());

    if (!dir) {
        return false;
    }

    struct dirent* entry;
    bool found = false;

    while ((entry = readdir(dir)) != nullptr) {
        string filename = entry->d_name;

        if (filename.find("pin") == 0 && filename.rfind(".code") == filename.size() - 5) {
            found = true;
            break;
        }
    }

    closedir(dir);
    return found;
}

void firstTimeUsers(const string& DRIVE, Bank& bank) {
    if (!pinFileExists(DRIVE)) {
        NewAccount open;
        setGreen();
        cout << "No PIN file found. Setting up a new PIN.\n";
        resetColor();
        open.openAccount(bank, DRIVE);  
        bank.save(FILEPATH);
        setGreen();
        cout << "\nSetup complete! Please restart the program and insert your card to login.\n";
        resetColor();
        system("pause");
        exit(0);
    }
}

bool isDriveInserted(const string& drive) {
    DWORD driveType = GetDriveTypeA((drive + "\\").c_str());
    return driveType != DRIVE_NO_ROOT_DIR;
}

void waitForCard(const string& drive) {
    setGreen();
    cout << "Please insert your card..." << endl;
    resetColor();

    while (!isDriveInserted(drive)) {
        Sleep(1000); 
    }
}

void realTime(){
    auto now = chrono::system_clock::now();
    time_t curTime = chrono::system_clock::to_time_t(now);
    tm phTime = *std::localtime(&curTime);

    setGreen();
    cout << "Date: " << std::put_time(&phTime, "%Y-%m-%d") << endl;
    cout << "Time: " << std::put_time(&phTime, "%H:%M:%S") << endl;
    resetColor();
}

void transactionDisplay(int i){
    string display[] = {"Balance Inquiry", "Withdraw",
                        "Deposit", "Fund Transfer",
                        "Change PIN Code", "View Accounts"
                        };
    int s = sizeof(display) / sizeof(display[0]);
    setGreen();
    if(i >= 0 && i < s){
        cout << "=== " << display[i] << " ===" << endl;
    } else{
       cout << "404 Not Found." << endl;
    }
    resetColor();
}

void Bank::save(const string& filePath) {
    ofstream fout(filePath, ios::out);
    if (!fout) {
        setGreen();
        cout << "Error: Cannot open file to save data.\n";
        resetColor();
        return;
    }

    DataBase* cur = head;
    while (cur != NULL) {
        Account* acc = cur->infos.newAccount;
        while (acc != NULL) {
            fout << acc->accountNo << "|"
                 << cur->infos.accountName << "|"
                 << cur->infos.birthday << "|"
                 << cur->infos.contactNo << "|"
                 << acc->balance << "|"
                 << acc->pinCode << "|"
                 << acc->accType << "\n"; 
            acc = acc->next;
        }
        cur = cur->next;
    }

    fout.close();
}

void Bank::load(const string& filePath) {
    ifstream fin(filePath, ios::in);
    if (!fin) {
        return;
    }

    string line;
    while (getline(fin, line)) {
        stringstream ss(line);
        CustomersData data;
        Account acc;
        string token;

        getline(ss, acc.accountNo, '|');
        getline(ss, data.accountName, '|');
        getline(ss, data.birthday, '|');
        getline(ss, data.contactNo, '|');
        getline(ss, token, '|');  
        acc.balance = stod(token);
        getline(ss, acc.pinCode, '|');
        getline(ss, acc.accType, '|');

        DataBase* existing = head;
        DataBase* found = NULL;
        while (existing != NULL) {
            if (existing->infos.accountName == data.accountName) {
                found = existing;
                break;
            }
            existing = existing->next;
        }

        if (found != NULL) {
            Account* newAcc = new Account{acc.accountNo, acc.pinCode, acc.balance, acc.accType, NULL};
            Account* temp = found->infos.newAccount;
            if (temp == NULL) {
                found->infos.newAccount = newAcc;
            } else {
                while (temp->next != NULL) temp = temp->next;
                temp->next = newAcc;
            }
        } else {
            DataBase* newNode = new DataBase(data);
            newNode->infos.newAccount = new Account{acc.accountNo, acc.pinCode, acc.balance, acc.accType, NULL};
            if (head == NULL) {
                head = newNode;
                tail = newNode;
            } else {
                tail->next = newNode;
                newNode->prev = tail;
                tail = newNode;
            }
        }
    }

    fin.close();
}

void Bank::balanceInquiry(string accNo){
    DataBase* result = findAccount(accNo);

    if(result != NULL) {
        Account* acc = result->infos.newAccount;
        while (acc != NULL) {
            if (acc->accountNo == accNo) {
                setGreen();
                cout << "Balance: ₱" << fixed << setprecision(2) << acc->balance << endl;
                resetColor();
                realTime();
                setGreen();
                cout << "Details: Checked Balance." << endl;
                resetColor();
                return;
            }
            acc = acc->next;
        }
    } else {
        setGreen();
        cout << "Account not found." << endl;
        resetColor();
    }
}

bool hasSufficiency(double amount, DataBase* cur){
    if(amount <= 0){
        setGreen();
        cout << "Invalid amount. Must be greater than zero." << endl;
        resetColor();
        return false;
    }
    if(amount > 15000){
        setGreen();
        cout << "Exceeds withdraw limits [15,000]." << endl;
        resetColor();
        return false;
    }
    if(cur->infos.newAccount->balance < amount){
        setGreen();
        cout << "Insufficient funds." << endl;
        resetColor();
        return false;
    }
    return true;
}

void Bank::withdraw(double getAmount, DataBase* cur){
    try{
        if(!hasSufficiency(getAmount, cur)){
            throw invalid_argument("Withdrawal validation failed.");
        }
        
        cur->infos.newAccount->balance -= getAmount;
        cout << fixed << setprecision(2);
        setGreen();
        cout << "Amount withdrawn: ₱" << getAmount << endl;
        cout << "Current Balance: ₱" << cur->infos.newAccount->balance << endl;
        resetColor();
        realTime();
        setGreen();
        cout << "Status: Withdrawal successful." << endl;
        resetColor();
    } catch (const exception& err) {
        setGreen();
        cout << "Error: " << err.what() << endl;
        resetColor();
    }
}

bool isValidDeposit(double amount){
    return amount > 0;
}

bool isValidInitialDeposit(double amount){
   return amount >= 5000;
}

void Bank::deposit(double depositAmount, DataBase* cur){
    try{
        if(!isValidDeposit(depositAmount)){
            throw invalid_argument("Can not perform transaction because of invalid amount");
        }

        cur->infos.newAccount->balance += depositAmount;

        if(cur->infos.newAccount->balance == depositAmount){
           if(!isValidInitialDeposit(depositAmount)){
               throw invalid_argument("Required minimum of ₱5,000 initial deposit");
           }
        }

        cout << fixed << setprecision(2);
        setGreen();
        cout << "Deposit amount: ₱" << depositAmount << endl;
        cout << "Current Balance: ₱" << cur->infos.newAccount->balance << endl;
        resetColor();
        realTime();
        setGreen();
        cout << "Status: Successfully deposited." << endl;
        resetColor();

    } catch (const exception& err){
        setGreen();
        cout << "Error: " << err.what() << endl;
        resetColor();
    }
}

bool canTransfer(DataBase* sender, double amount, string recieverNo){
    if(amount <= 0){
        setGreen();
        cout << "Invalid transfer amount." << endl;
        resetColor();
        return false;
    }
    if(amount > sender->infos.newAccount->balance){
        setGreen();
        cout << "Insufficient funds to transfer." << endl;
        resetColor();
        return false;
    }
    if(sender->infos.newAccount->accountNo == recieverNo){
        setGreen();
        cout << "Cannot transfer to the same account." << endl;
        resetColor();
        return false;
    }
    return true;
}

DataBase* Bank::findAccount(const string& recieverNo){
    DataBase* reciever = head;

    while(reciever != NULL){
        Account* acc = reciever->infos.newAccount;
        while (acc != NULL) {
            if (acc->accountNo == recieverNo)
                return reciever;
            acc = acc->next;
        }
        reciever = reciever->next;
    }
    return NULL;
}

void Bank::fundTransfer(double amount, DataBase* sender, string recieverNo){
    try{
        if(!canTransfer(sender, amount, recieverNo)){
            throw invalid_argument("Cannot perform fund transfer because of insuffiency");
        }

        DataBase* reciever = findAccount(recieverNo);
        if(!reciever)
            throw runtime_error("Reciever account not found.");

        Account* receiverAcc = reciever->infos.newAccount;
        while (receiverAcc != NULL) {
            if (receiverAcc->accountNo == recieverNo) {
                sender->infos.newAccount->balance -= amount;
                receiverAcc->balance += amount;
                break;
            }
            receiverAcc = receiverAcc->next;
        }
        
        realTime();
        setGreen();
        cout << "Receiver Account: " << recieverNo << endl;
        cout << fixed << setprecision(2);
        cout << "Amount Transferred: ₱" << amount << endl;
        cout << "Current balance: ₱" << sender->infos.newAccount->balance << endl; 
        cout << "Status: Transfer Successful" << endl;
        resetColor();
    } catch (const exception& err){
        setGreen();
        cout << "Error: "  << err.what() << endl;
        resetColor();
    }
}

bool isValidFormat(DataBase* account, const string& oldPin, const string& newPin, const string& confirmPin){
    if(!account){
        setGreen();
        cout << "Account not found." << endl;
        resetColor();
        return false;
     }
    if(account->infos.newAccount->pinCode != oldPin){
        setGreen();
        cout << "Incorrect entered PIN." << endl;
        resetColor();
        return false;
    }
    if(newPin.length() != 4 && newPin.length() != 6){
        setGreen();
        cout << "PIN must contains 4 or 6 digits." << endl;
        resetColor();
        return false;
    }
    if(!all_of(newPin.begin(), newPin.end(), ::isdigit)){
        setGreen();
        cout << "PIN must contains digits only." << endl;
        resetColor();
        return false;
    }
    if(newPin != confirmPin){
        setGreen();
        cout << "PIN confirmation does not match" << endl;
        resetColor();
        return false;
    }

    return true;
}

void Bank::changePin(DataBase* account, const string& oldPin, const string& newPin, const string& confirmPin){
    try{ 
        if(!isValidFormat(account, oldPin, newPin, confirmPin)){
            throw invalid_argument("Can not perform change pin because of invalid actions");
        } else {
            
            Account* acc = account->infos.newAccount;
            Security sec;
            string updatedPin = sec.encrypt(newPin);
            bool isFound = false;

            while(acc != NULL){
                if(acc->pinCode == oldPin){
                    acc->pinCode = updatedPin;
                    isFound = true;
                    break;
                }
                acc = acc->next;
            }
            if(isFound){
                time_t now = time(0);
                tm* phTime = localtime(&now);
                setGreen();
                cout << "PIN updated on: " << put_time(phTime, "%Y-%m-%d %H:%M:%S") << endl;
                cout << "Success: Your PIN has been changed." << endl;
                resetColor();

            } else {
                setGreen();
                cout << "Error: Old PIN not found or incorrect.\n";
                resetColor();
            }
        }
        
      
    } catch (const exception& err){
        setGreen();
        cout << "Error: " << err.what() << endl;
        resetColor();
    }
}

string Security::getPinCode(){
    string pinCode = "";
    char c;

    setGreen();
    cout << "Enter PIN [only digits allowed]: ";
    resetColor();
    while ((c = _getch()) != '\r' )
    {
        if(c >= '0' && c <= '9'){
            pinCode += c;
            setGreen();
            cout << "*";
            resetColor();
        } else if(c == '\b') {
            if(!pinCode.empty()){
                cout << "\b \b";
                pinCode.pop_back();
            }
        }
    }
    
    cout << endl;
    system("pause");
    return pinCode;
}

string Security::encrypt(string pin){
    for(size_t i = 0; i < pin.size(); i++){
        pin[i] = pin[i] + v;
    }
    return pin;
}

string Security::decrypt(string pin){
    for(size_t i = 0; i < pin.size(); i++){
        pin[i] = pin[i] - v;
    }
    return pin;
}

void Security::savePin(const string& pin, const string& DRIVE, const string& accNo){
    string filepath = DRIVE + "/Private/pin"+ accNo +".code";
    try{
        ofstream fout(filepath, ios::binary);
        if(!fout){
            throw invalid_argument("Flash drive not found at " + filepath);
        }
        fout << pin;
        fout.close();
        setGreen();
        cout << "PIN saved to " << filepath << endl;
        resetColor();
    } catch(const invalid_argument& err){
        setGreen();
        cout << "Exception found: " << err.what() << endl;
        resetColor();
    }
}

bool Security::verifyPin(Account* acc, const string& inputPin, const string& DRIVE) {
    if (!acc || acc->pinCode.empty()) {
        return false;
    }

    string filepath = DRIVE + "/Private/pin" + acc->accountNo + ".code";
    ifstream fin(filepath, ios::binary);
    if (!fin) {
        setGreen();
        cout << "Please insert card." << endl;
        resetColor();
        return false;
    }

    string encryptedCardPin;
    getline(fin, encryptedCardPin);
    fin.close();

    string decryptedCardPin = decrypt(encryptedCardPin);
    string decryptedBankPin = decrypt(acc->pinCode);
    
    bool cardMatch = (inputPin == decryptedCardPin);
    bool bankMatch = (inputPin == decryptedBankPin);

    return cardMatch && bankMatch;
}

bool isValidPinFormat(const string& pin){
    if(pin.size() < 4 && pin.size() > 6){
        setGreen();
        cout << "PIN must contains 4 or 6 digits." << endl;
        resetColor();
        return false;
    }
    if(pin.empty()){
        setGreen();
        cout << "PIN must not be empty." << endl;
        resetColor();
        return false;
    }
    if(!all_of(pin.begin(), pin.end(), ::isdigit)) {
        setGreen();
        cout << "PIN must contains digits only." << endl;
        resetColor();
        return false;
    }
    return true;
}

void NewAccount::createPin(CustomersData& d, const string DRIVE, string pin){
    Security sec;
    try {
        if(!isValidPinFormat(pin))
            throw invalid_argument("Invalid pin number causing error.");
        
        Account* newAcc = new Account{"", "", 0, "", nullptr};
        
        string encryptedPin = sec.encrypt(pin);
        newAcc->pinCode = encryptedPin;
        savePin(encryptedPin, DRIVE, newAcc->accountNo);
        
        if(!d.newAccount){
            d.newAccount = newAcc;
        } else{
            Account *ptr = d.newAccount;
            while(ptr->next != NULL)
                ptr = ptr->next;
            ptr->next = newAcc;
        }

        setGreen();
        cout << "PIN setup complete." << endl;
        resetColor();
    } catch (const invalid_argument& err){
        setGreen();
        cout << "Exception Found:" << err.what() << endl;
        resetColor();
    }
}

bool accNoExists(const string& accNo, DataBase* head) {
    DataBase* cur = head;
    while (cur != NULL) {
        Account* acc = cur->infos.newAccount;
        while (acc != NULL) {
            if (acc->accountNo == accNo)
                return true;
            acc = acc->next;
        }
        cur = cur->next;
    }
    return false;
}


string accNoGenerator(DataBase* head){
    srand(static_cast<unsigned int>(time(0)));
    string accNo;
    bool isExists;

    do {
        int randInt = rand() % 90000 + 10000;
        accNo = to_string(randInt);
        isExists = accNoExists(accNo, head);
    }while(isExists);
    return accNo;
}

string chooseAccountType() {
    int choice;
    setGreen();
    cout << "\nChoose account type:\n";
    cout << "1. Savings Account\n";
    cout << "2. Retirement Fund\n";
    cout << "3. Checking Account\n";
    cout << "4. Time Deposit\n";
    cout << "5. Student Account\n";
    cout << "Enter your choice: ";
    resetColor();
    cin >> choice;

    switch (choice) {
        case 1: return "Savings";
        case 2: return "Retirement Fund";
        case 3: return "Checking";
        case 4: return "Time Deposit";
        case 5: return "Student";
        default:
            setGreen();
            cout << "Invalid choice. Defaulting to Savings Account.\n";
            resetColor();
            return "Savings";
    }
}

void NewAccount::createAccNo(CustomersData& d,  const string& pin, double initDeposit, const string& accounType){
    Security sec;

    try{
        string accNo = accNoGenerator(getHead());
       
        if (pin != d.newAccount->pinCode) {
            throw invalid_argument("PIN does not match the one previously created.");
        }

        string accountType = chooseAccountType();
        string encryptedPin = sec.encrypt(pin);
        addAccount(d, accNo, encryptedPin, initDeposit, accounType);
        setGreen();
        cout << "Your " << accountType << " account has been successfully created.\n";
        cout << "Your account has been successfully created.";
        resetColor();
        
    } catch (const invalid_argument& err) {
        setGreen();
        cout << "Exeception Found:" << err.what() << endl;
        resetColor();
    }
}

void NewAccount::addAccount(CustomersData &customer, const string& accNo, const string& pin, double initDeposit, const string& accountType){
    Account* newAcc = new Account{accNo, pin, initDeposit, accountType, NULL};
    
    if(customer.newAccount == NULL){
        customer.newAccount = newAcc;
    } else {
        Account* buff = customer.newAccount;
        while(buff->next != NULL)
            buff = buff->next;
        buff->next = newAcc;
    }
    setGreen();
    cout << "Welcome to " << BANKNAME << " your new account " << accNo << " added for " << customer.accountName << endl;
    resetColor();
}

void Bank::addToDatabase(DataBase* newNode) {
    if (head == NULL) {
        head = newNode;
        tail = newNode;
    } else {
        tail->next = newNode;
        newNode->prev = tail;
        tail = newNode;
    }
}

void NewAccount::openAccount(Bank& bank, const string& DRIVE){
    CustomersData customer;
    customer.newAccount = nullptr;
    
    setGreen();
    cout << "=== Account Registration ===\n";
    cout << "\nChoose registration type:\n";
    cout << "1. Add account to existing customer\n";
    cout << "2. Create new customer profile\n";
    cout << "3. Create new customer profile\n";
    cout << "Enter choice: ";
    resetColor();
    
    int regChoice;
    cin >> regChoice;
    cin.ignore();

    if(regChoice == 1){
        // Adding account to existing customer (must be logged-in user)
        string currentAccNo = bank.getCurrentAccount();
        if(currentAccNo.empty()){
            setGreen();
            cout << "Error: No logged in account. Cannot add account.\n";
            resetColor();
            return;
        }

        // Find the logged-in user
        DataBase* loggedInUser = bank.findAccount(currentAccNo);
        if(!loggedInUser){
            setGreen();
            cout << "Error: Logged in account not found.\n";
            resetColor();
            return;
        }

        setGreen();
        cout << "\nAdding new account for: " << loggedInUser->infos.accountName << "\n";
        resetColor();

        string pin;
        setGreen();
        cout << "Create PIN for new account (4 or 6 digits): ";
        resetColor();
        cin >> pin;
        
        if(!isValidPinFormat(pin)){
            setGreen();
            cout << "Invalid PIN format. Account creation cancelled.\n";
            resetColor();
            return;
        }
        
        Security sec;
        string encryptedPin = sec.encrypt(pin);
        
        setGreen();
        cout << "PIN setup complete.\n";
        resetColor();

        string generatedAccPin = accNoGenerator(bank.getHead());
        setGreen();
        cout << "New Account Number: " << generatedAccPin << "\n";
        resetColor();
       
        sec.savePin(encryptedPin, DRIVE, generatedAccPin);

        double initDeposit;
        setGreen();
        cout << "Enter Initial Deposit (min ₱5000): ";
        resetColor();
        cin >> initDeposit;

        if(initDeposit < 5000){
            setGreen();
            cout << "Initial deposit must be at least ₱5,000.\n";
            resetColor();
            return;
        }

        string accountType = chooseAccountType();

        // Add account to existing customer
        Account* newAcc = new Account{generatedAccPin, encryptedPin, initDeposit, accountType, nullptr};
        Account* temp = loggedInUser->infos.newAccount;
        while(temp->next) temp = temp->next;
        temp->next = newAcc;

        setGreen();
        cout << "\nNew account successfully attached!\n";
        cout << "Account Number: " << generatedAccPin << endl;
        cout << "Account Type: " << accountType << endl;
        cout << "Balance: ₱" << fixed << setprecision(2) << initDeposit << endl;
        resetColor();
        realTime();
        return;
    }
    else if(regChoice == 2){
        // Creating completely new customer profile
        setGreen();
        cout << "\n=== New Customer Registration ===\n";
        cout << "Enter Full Name: ";
        resetColor();
        getline(cin, customer.accountName);
        setGreen();
        cout << "Enter Birthday [YYYY-MM-DD]: ";
        resetColor();
        getline(cin, customer.birthday);
        setGreen();
        cout << "Enter Contact Number: ";
        resetColor();
        getline(cin, customer.contactNo);

        string pin;
        setGreen();
        cout << "Create PIN (4 or 6 digits): ";
        resetColor();
        cin >> pin;
        
        if(!isValidPinFormat(pin)){
            setGreen();
            cout << "Invalid PIN format. Account creation cancelled.\n";
            resetColor();
            return;
        }
        
        Security sec;
        string encryptedPin = sec.encrypt(pin);
        
        setGreen();
        cout << "PIN setup complete.\n";
        resetColor();

        string generatedAccPin = accNoGenerator(bank.getHead());
        setGreen();
        cout << "Account Number: " << generatedAccPin << "\n";
        resetColor();
       
        sec.savePin(encryptedPin, DRIVE, generatedAccPin);

        double initDeposit;
        setGreen();
        cout << "Enter Initial Deposit (min ₱5000): ";
        resetColor();
        cin >> initDeposit;

        if(initDeposit < 5000){
            setGreen();
            cout << "Initial deposit must be at least ₱5,000.\n";
            resetColor();
            return;
        }

        string accountType = chooseAccountType();

        // Create new customer in database
        customer.newAccount = new Account{generatedAccPin, encryptedPin, initDeposit, accountType, nullptr};

        DataBase* newNode = new DataBase(customer);
        bank.addToDatabase(newNode);

        setGreen();
        cout << "\nAccount successfully created!\n";
        cout << "Welcome, " << customer.accountName << "!\n";
        cout << "Account Number: " << generatedAccPin << endl;
        cout << "Account Type: " << accountType << endl;
        cout << "Balance: ₱" << fixed << setprecision(2) << initDeposit << endl;
        cout << "\nNote: This is a separate customer profile. To access this account,\n";
        cout << "you will need to log in with the new account credentials.\n";
        resetColor();
        realTime();
    }
    else{
        setGreen();
        cout << "Cancel plan transaction\n";
        resetColor();
    }
}

void Bank::viewAccounts(){
    if(currentLoggedInAccount.empty()){
        setGreen();
        cout << "No logged in account detected.\n";
        resetColor();
        return;
    }

    // Find the logged-in user's customer data
    DataBase* loggedInUser = nullptr;
    DataBase* cur = head;
    
    while(cur != NULL){
        Account* acc = cur->infos.newAccount;
        while(acc != NULL){
            if(acc->accountNo == currentLoggedInAccount){
                loggedInUser = cur;
                break;
            }
            acc = acc->next;
        }
        if(loggedInUser) break;
        cur = cur->next;
    }

    if(!loggedInUser){
        setGreen();
        cout << "Error: Logged in account not found.\n";
        resetColor();
        return;
    }

    // Display only accounts belonging to this user
    setGreen();
    cout << "\n=== My Accounts ===\n";
    cout << "Account Holder: " << loggedInUser->infos.accountName << "\n";
    cout << "Birthday: " << loggedInUser->infos.birthday << "\n";
    cout << "Contact: " << loggedInUser->infos.contactNo << "\n\n";
    
    cout << left << setw(15) << "Account No" 
         << setw(18) << "Account Type"
         << setw(15) << "Balance"
         << setw(10) << "Status" << endl;
    cout << string(58, '-') << endl;
    resetColor();

    Account* acc = loggedInUser->infos.newAccount;
    while(acc != NULL){
        if(acc->accountNo == currentLoggedInAccount){
            
            setYellow();
            cout << left << setw(15) << acc->accountNo
                 << setw(18) << acc->accType
                 << "₱" << fixed << setprecision(2) << setw(14) << acc->balance
                 << setw(10) << "[ACTIVE]" << endl;
            resetColor();
        } else {
            setGreen();
            cout << left << setw(15) << acc->accountNo
                 << setw(18) << acc->accType
                 << "₱" << fixed << setprecision(2) << setw(14) << acc->balance
                 << setw(10) << "" << endl;
            resetColor();
        }
        acc = acc->next;
    }
    cout << endl;
}

int bankInterface(){
    int choice;
    setGreen();
    std::cout << R"(
    /$$    /$$$$$$$   /$$$$$$  /$$   /$$ /$$   /$$
  /$$$$$$ | $$__  $$ /$$__  $$| $$$ | $$| $$  /$$/
 /$$__  $$| $$  \ $$| $$  \ $$| $$$$| $$| $$ /$$/ 
| $$  \__/| $$$$$$$ | $$$$$$$$| $$ $$ $$| $$$$$/  
|  $$$$$$ | $$__  $$| $$__  $$| $$  $$$$| $$  $$  
 \____  $$| $$  \ $$| $$  | $$| $$\  $$$| $$\  $$ 
 /$$  \ $$| $$$$$$$/| $$  | $$| $$ \  $$| $$ \  $$
|  $$$$$$/|_______/ |__/  |__/|__/  \__/|__/  \__/
 \_  $$_/                                         
   \__/                                           
                                                  )" << std::endl;
    
    cout << "1. Add Account" << endl << "2. Deposit" << endl << "3. Withdraw" 
    << endl << "4. Transfer" << endl << "5. Change PIN" << endl 
    << "6. View Accounts" << endl << "7. Bank Inquiry" << endl <<
     "8. Exit" << endl << "Select [1-8]: ";
    resetColor();
    cin >> choice;
    return choice;
}