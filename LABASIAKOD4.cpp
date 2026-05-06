#include <iostream>
#include <string>
#include <functional>
#include <vector>
#include <algorithm>
#include <chrono>
#include <iomanip>

using namespace std;
using namespace chrono;

// ---------- Структура данных об автобусе ----------
struct Bus {
    int number;             // номер автобуса (уникальный ключ)
    string driverName;      // ФИО водителя
    int routeNumber;        // номер маршрута
    bool onRoute;           // true - на маршруте, false - в парке

    // Конструктор для удобства
    Bus(int num = 0, const string& driver = "", int route = 0, bool routeStatus = false)
        : number(num), driverName(driver), routeNumber(route), onRoute(routeStatus) {}

    // Оператор сравнения для сортировки (по номеру)
    bool operator<(const Bus& other) const { return number < other.number; }
    bool operator>(const Bus& other) const { return number > other.number; }
    bool operator==(const Bus& other) const { return number == other.number; }
};

// ---------- Узел бинарного дерева поиска (по номеру автобуса) ----------
struct TreeNode {
    Bus data;
    TreeNode* left;
    TreeNode* right;
    TreeNode(const Bus& b) : data(b), left(nullptr), right(nullptr) {}
};

// ---------- Класс бинарного дерева поиска ----------
class BinaryTree {
private:
    TreeNode* root;

    // Вспомогательная рекурсивная вставка
    TreeNode* insert(TreeNode* node, const Bus& bus) {
        if (!node) return new TreeNode(bus);
        if (bus.number < node->data.number)
            node->left = insert(node->left, bus);
        else if (bus.number > node->data.number)
            node->right = insert(node->right, bus);
        // если равны — не вставляем (номер уникален)
        return node;
    }

    // Поиск узла по номеру
    TreeNode* search(TreeNode* node, int number) const {
        if (!node || node->data.number == number) return node;
        if (number < node->data.number) return search(node->left, number);
        else return search(node->right, number);
    }

    // Удаление узла по номеру (стандартный алгоритм)
    TreeNode* remove(TreeNode* node, int number) {
        if (!node) return nullptr;
        if (number < node->data.number)
            node->left = remove(node->left, number);
        else if (number > node->data.number)
            node->right = remove(node->right, number);
        else {
            // узел найден
            if (!node->left && !node->right) {
                delete node;
                return nullptr;
            }
            else if (!node->left) {
                TreeNode* temp = node->right;
                delete node;
                return temp;
            }
            else if (!node->right) {
                TreeNode* temp = node->left;
                delete node;
                return temp;
            }
            else {
                // два потомка: ищем минимальный в правом поддереве
                TreeNode* minNode = node->right;
                while (minNode->left) minNode = minNode->left;
                node->data = minNode->data;
                node->right = remove(node->right, minNode->data.number);
            }
        }
        return node;
    }

    // Рекурсивный обход для вывода всех автобусов (симметричный)
    void inorderPrint(TreeNode* node) const {
        if (!node) return;
        inorderPrint(node->left);
        printBus(node->data);
        inorderPrint(node->right);
    }

    // Рекурсивный обход для поиска по признаку (на маршруте/в парке)
    void findByStatus(TreeNode* node, bool onRoute, vector<Bus>& result) const {
        if (!node) return;
        findByStatus(node->left, onRoute, result);
        if (node->data.onRoute == onRoute)
            result.push_back(node->data);
        findByStatus(node->right, onRoute, result);
    }

    // Рекурсивное удаление всего дерева
    void clear(TreeNode* node) {
        if (!node) return;
        clear(node->left);
        clear(node->right);
        delete node;
    }

public:
    BinaryTree() : root(nullptr) {}
    ~BinaryTree() { clear(root); }

    void insert(const Bus& bus) { root = insert(root, bus); }

    Bus* search(int number) {
        TreeNode* node = search(root, number);
        return node ? &(node->data) : nullptr;
    }

    void remove(int number) { root = remove(root, number); }

    void printAll() const {
        if (!root) { cout << "Дерево пусто.\n"; return; }
        inorderPrint(root);
    }

    // Изменение статуса (выезд/возврат)
    bool setStatus(int number, bool onRoute) {
        Bus* bus = search(number);
        if (!bus) return false;
        bus->onRoute = onRoute;
        return true;
    }

    // Поиск автобусов по признаку (true = на маршруте, false = в парке)
    vector<Bus> findBusesByStatus(bool onRoute) const {
        vector<Bus> result;
        findByStatus(root, onRoute, result);
        return result;
    }

    // Получить все автобусы (для линейной структуры)
    vector<Bus> getAllBuses() const {
        vector<Bus> result;
        function<void(TreeNode*)> collect = [&](TreeNode* node) {
            if (!node) return;
            collect(node->left);
            result.push_back(node->data);
            collect(node->right);
        };
        collect(root);
        return result;
    }

private:
    void printBus(const Bus& b) const {
        cout << "Номер: " << b.number
             << ", Водитель: " << b.driverName
             << ", Маршрут: " << b.routeNumber
             << ", Статус: " << (b.onRoute ? "На маршруте" : "В парке") << endl;
    }
};

// ---------- Функции для работы с линейной структурой (вектор) ----------
// Линейный поиск по номеру
int linearSearch(const vector<Bus>& arr, int number) {
    for (size_t i = 0; i < arr.size(); ++i)
        if (arr[i].number == number) return i;
    return -1;
}

// Бинарный поиск (требует сортировки по номеру)
int binarySearch(const vector<Bus>& arr, int number) {
    int left = 0, right = arr.size() - 1;
    while (left <= right) {
        int mid = left + (right - left) / 2;
        if (arr[mid].number == number) return mid;
        else if (arr[mid].number < number) left = mid + 1;
        else right = mid - 1;
    }
    return -1;
}

// ---------- Вспомогательные функции для ввода/вывода ----------
Bus inputBus() {
    Bus b;
    cout << "Номер автобуса: "; cin >> b.number;
    cin.ignore();
    cout << "ФИО водителя: "; getline(cin, b.driverName);
    cout << "Номер маршрута: "; cin >> b.routeNumber;
    int status;
    cout << "Где находится? (1 - на маршруте, 0 - в парке): "; cin >> status;
    b.onRoute = (status == 1);
    return b;
}

void printBusList(const vector<Bus>& buses) {
    if (buses.empty()) {
        cout << "Нет автобусов.\n";
        return;
    }
    for (const auto& b : buses) {
        cout << "Номер: " << b.number
             << ", Водитель: " << b.driverName
             << ", Маршрут: " << b.routeNumber
             << ", Статус: " << (b.onRoute ? "На маршруте" : "В парке") << endl;
    }
}

// ---------- Анализ времени поиска ----------
void performanceAnalysis(BinaryTree& tree, const vector<Bus>& linearArraySorted) {
    // Выбираем автобус для поиска (например, первый из дерева)
    Bus* testBus = tree.search(linearArraySorted[0].number);
    if (!testBus) {
        cout << "Ошибка: нет автобусов для анализа.\n";
        return;
    }
    int testNumber = testBus->number;

    // 1. Поиск в дереве по номеру
    auto start = high_resolution_clock::now();
    volatile Bus* result1 = tree.search(testNumber); // volatile чтобы не оптимизировало
    auto end = high_resolution_clock::now();
    auto timeTreeByNumber = duration_cast<nanoseconds>(end - start).count();

    // 2. Поиск в дереве по признаку (на маршруте) – для сравнения, ищем тот же автобус среди всех с onRoute=true
    //    Но признак не уникален, поэтому просто замеряем обход
    start = high_resolution_clock::now();
    vector<Bus> onRouteBuses = tree.findBusesByStatus(true);
    end = high_resolution_clock::now();
    auto timeTreeByStatus = duration_cast<nanoseconds>(end - start).count();

    // 3. Линейный поиск в массиве (несортированном, но для анализа используем отсортированный? Линейный работает одинаково)
    //    Для честности возьмём копию вектора, но линейный поиск по номеру можно делать в любом порядке.
    //    Используем исходный (несортированный) массив, который получаем из дерева.
    vector<Bus> linearArray = tree.getAllBuses(); // порядок не гарантирован (inorder даёт отсортированный, но мы перемешаем?)
    // Для чистоты эксперимента оставим как есть (inorder даёт сортированный, но линейный поиск это не улучшает).
    // Лучше перемешать, чтобы линейный поиск не выигрывал от случайной близости.
    // Но для демонстрации можно просто использовать тот же массив.

    start = high_resolution_clock::now();
    int idxLin = linearSearch(linearArray, testNumber);
    end = high_resolution_clock::now();
    auto timeLinear = duration_cast<nanoseconds>(end - start).count();

    // 4. Бинарный поиск в отсортированном массиве
    start = high_resolution_clock::now();
    int idxBin = binarySearch(linearArraySorted, testNumber);
    end = high_resolution_clock::now();
    auto timeBinary = duration_cast<nanoseconds>(end - start).count();

    // Вывод результатов
    cout << "\n========== СРАВНИТЕЛЬНЫЙ АНАЛИЗ ВРЕМЕНИ ПОИСКА ==========\n";
    cout << left << setw(35) << "Метод поиска" << setw(15) << "Время (нс)" << "Результат\n";
    cout << "------------------------------------------------------------\n";
    cout << left << setw(35) << "Дерево (по номеру)" << setw(15) << timeTreeByNumber << "найден\n";
    cout << left << setw(35) << "Дерево (по признаку 'на маршруте')" << setw(15) << timeTreeByStatus << (result1->onRoute ? "автобус найден в выборке" : "автобус не на маршруте") << endl;
    cout << left << setw(35) << "Линейный поиск в массиве" << setw(15) << timeLinear << (idxLin != -1 ? "найден" : "не найден") << endl;
    cout << left << setw(35) << "Бинарный поиск в отсортированном массиве" << setw(15) << timeBinary << (idxBin != -1 ? "найден" : "не найден") << endl;
    cout << "============================================================\n";
}

// ---------- Главное меню ----------
int main() {
    BinaryTree tree;
    vector<Bus> linearArray; // для линейных структур (будет обновляться при изменениях)

    int choice;
    do {
        cout << "\n=== АВТОБУСНЫЙ ПАРК ===\n";
        cout << "1. Начальное формирование данных\n";
        cout << "2. Добавить автобус\n";
        cout << "3. Удалить автобус\n";
        cout << "4. Изменить статус (выезд/возврат)\n";
        cout << "5. Вывести все автобусы\n";
        cout << "6. Вывести автобусы в парке\n";
        cout << "7. Вывести автобусы на маршруте\n";
        cout << "8. Сравнительный анализ времени поиска\n";
        cout << "0. Выход\n";
        cout << "Ваш выбор: ";
        cin >> choice;

        switch (choice) {
            case 1: {
                int n;
                cout << "Введите количество автобусов: ";
                cin >> n;
                for (int i = 0; i < n; ++i) {
                    cout << "Автобус " << i+1 << ":\n";
                    Bus b = inputBus();
                    tree.insert(b);
                }
                // обновляем линейный массив
                linearArray = tree.getAllBuses();
                cout << "Данные успешно загружены.\n";
                break;
            }
            case 2: {
                Bus b = inputBus();
                tree.insert(b);
                linearArray = tree.getAllBuses();
                cout << "Автобус добавлен.\n";
                break;
            }
            case 3: {
                int num;
                cout << "Введите номер автобуса для удаления: ";
                cin >> num;
                tree.remove(num);
                linearArray = tree.getAllBuses();
                cout << "Автобус удалён (если существовал).\n";
                break;
            }
            case 4: {
                int num, status;
                cout << "Введите номер автобуса: ";
                cin >> num;
                cout << "Новый статус (1 - на маршруте, 0 - в парке): ";
                cin >> status;
                if (tree.setStatus(num, status == 1))
                    cout << "Статус изменён.\n";
                else
                    cout << "Автобус не найден.\n";
                linearArray = tree.getAllBuses();
                break;
            }
            case 5: {
                cout << "\nСписок всех автобусов:\n";
                tree.printAll();
                break;
            }
            case 6: {
                vector<Bus> parked = tree.findBusesByStatus(false);
                cout << "\nАвтобусы в парке:\n";
                printBusList(parked);
                break;
            }
            case 7: {
                vector<Bus> onRoute = tree.findBusesByStatus(true);
                cout << "\nАвтобусы на маршруте:\n";
                printBusList(onRoute);
                break;
            }
            case 8: {
                // Подготовим отсортированный массив для бинарного поиска
                vector<Bus> sortedArray = tree.getAllBuses();
                sort(sortedArray.begin(), sortedArray.end());
                performanceAnalysis(tree, sortedArray);
                break;
            }
            case 0:
                cout << "До свидания!\n";
                break;
            default:
                cout << "Неверный выбор.\n";
        }
    } while (choice != 0);

    return 0;
}
