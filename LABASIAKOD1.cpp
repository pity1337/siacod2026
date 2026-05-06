#include <iostream>
#include <vector>
#include <string>
#include <queue>
#include <set>
#include <chrono>

using namespace std;
using namespace chrono;

// Своя очередь
class MyQueue {
    string* buf;        // указатель на динамический массив строк (буфер)
    size_t cap;         // текущая ёмкость буфера (сколько элементов может вместить)
    size_t head;        // индекс для чтения (начало очереди)
    size_t tail;        // индекс для записи (конец очереди)
    //  нет поля count Размер вычисляется через head и tail
    
    // Приватный метод для увеличения буфера в 2 раза при переполнении
    void resize() {
        size_t newCap = cap * 2;                    // новая ёмкость
        string* newBuf = new string[newCap];        // выделяем новый буфер
        
        // Копируем элементы в новый буфер последовательно
        if (head < tail) {
            // Простой случай: все элементы лежат подряд от head до tail
            for (size_t i = head; i < tail; ++i)
                newBuf[i - head] = std::move(buf[i]);   // перемещаем строки
        } else if (head > tail) {
            // Сначала копируем от head до конца буфера
            size_t idx = 0;
            for (size_t i = head; i < cap; ++i)
                newBuf[idx++] = std::move(buf[i]);
            // Потом копируем от начала до tail
            for (size_t i = 0; i < tail; ++i)
                newBuf[idx++] = std::move(buf[i]);
        }
        
        delete[] buf;                                   // освобождаем старый буфер
        buf = newBuf;                                   // переключаемся на новый
        // Пересчитываем tail для нового буфера (теперь все элементы подряд)
        tail = (head < tail) ? (tail - head) : (cap - head + tail);
        head = 0;                                        // head теперь в начале
        cap = newCap;                                    // обновляем ёмкость
    }

public:
    // Конструктор
    MyQueue(size_t initCap = 64) : cap(initCap), head(0), tail(0) { 
        buf = new string[cap];                           // выделяем память под буфер
    }
    
    // Запрещаем копирование
    MyQueue(const MyQueue&) = delete;
    MyQueue& operator=(const MyQueue&) = delete;
    
    // Разрешаем перемещение 
    // Конструктор перемещения
    
    MyQueue(MyQueue&& other) noexcept 
        : buf(other.buf), cap(other.cap), head(other.head), tail(other.tail) {
        // Забираем ресурсы у other
        other.buf = nullptr;    // other больше не владеет буфером
        other.cap = 0;           // обнуляем его поля
        other.head = 0;
        other.tail = 0;
    }
    
    // Оператор присваивания перемещением
    MyQueue& operator=(MyQueue&& other) noexcept {
        if (this != &other) {                            // защита от самоприсваивания
            delete[] buf;                                 // освобождаем свой буфер
            buf = other.buf;                              // забираем буфер other
            cap = other.cap;
            head = other.head;
            tail = other.tail;
            other.buf = nullptr;                          // other больше не владеет
            other.cap = 0;
            other.head = 0;
            other.tail = 0;
        }
        return *this;
    }
    
    // Деструктор - освобождаем память
    ~MyQueue() { 
        delete[] buf;                                     // удаляем буфер
    }
    
    // Добавление элемента в конец очереди
    void push(const string& s) {
        size_t newTail = (tail + 1) % cap;                // следующая позиция для записи
        if (newTail == head) {                             // если буфер заполнен
            resize();                                       // расширяем его
            newTail = (tail + 1) % cap;                     // пересчитываем newTail
        }
        buf[tail] = s;                                      // копируем строку
        tail = newTail;                                     // обновляем tail
    }
    
    // Удаление элемента из начала очереди
    void pop() {
        if (head != tail) {                                 // если очередь не пуста
            // явно вызываем деструктор строки
            buf[head].~basic_string();                      // освобождаем ресурсы строки
            // Создаём новую пустую строку на том же месте 
            new (&buf[head]) string();
            head = (head + 1) % cap;                        // сдвигаем head
        }
    }
    
    // Доступ к первому элементу (без удаления)
    string& front() { 
        return buf[head];                                   // возвращаем элемент по индексу head
    }
    
    // Проверка, пуста ли очередь
    bool empty() const { 
        return head == tail;                                // пуста, если индексы совпадают
    }
    
    // Метод для быстрого обмена содержимым двух очередей
    void swap(MyQueue& other) noexcept {
        std::swap(buf, other.buf);                         // обмениваем указатели на буферы
        std::swap(cap, other.cap);                         // обмениваем ёмкости
        std::swap(head, other.head);                       // обмениваем индексы
        std::swap(tail, other.tail);                       // обмениваем индексы
    }
};

// Специализация функции swap для MyQueue
namespace std {
    template<>
    void swap(MyQueue& a, MyQueue& b) noexcept {
        a.swap(b);                                          // вызываем наш метод swap
    }
}

// Функция поиска детей для заданного родителя (линейный перебор)
vector<string> childrenOf(const vector<pair<string,string>>& rel, const string& parent) {
    vector<string> ch;                                      // вектор для результатов
    ch.reserve(4);                                          // резервируем память 
    for (auto& p : rel)                                     // проходим по всем отношениям
        if (p.first == parent)                              // если родитель совпадает
            ch.push_back(p.second);                         // добавляем ребёнка
    return ch;                                              // возвращаем список детей
}

// Шаблонная функция для двух очередей (тип очереди задаётся шаблоном)
template<typename Queue>
vector<vector<string>> descendants2Q(const vector<pair<string,string>>& rel,
                                     const string& target,
                                     long long& timeNs) {
    // Засекаем время начала
    auto start = high_resolution_clock::now();
    
    vector<vector<string>> res;        // результат
    set<string> visited;                // множество посещённых имён 
    
    // Две очереди: текущий уровень и следующий уровень
    Queue cur, next;
    
    cur.push(target);                   // начинаем с целевого имени
    visited.insert(target);              // помечаем как посещённое

    // Пока есть элементы на текущем уровне
    while (!cur.empty()) {
        vector<string> curChildren;      // дети, найденные на этом уровне
        curChildren.reserve(16);          // резервируем память (предполагаем до 16 детей)
        
        // Обрабатываем всех на текущем уровне
        while (!cur.empty()) {
            // Берём имя из очереди и сразу перемещаем 
            string p = std::move(cur.front());
            cur.pop();                    // удаляем из очереди
            
            // Находим всех детей этого родителя
            for (const string& c : childrenOf(rel, p)) {
                // Если ребёнок ещё не посещён
                if (visited.find(c) == visited.end()) {
                    visited.insert(c);     // помечаем как посещённого
                    next.push(c);          // добавляем в очередь следующего уровня
                    curChildren.push_back(c); // запоминаем для результата
                }
            }
        }
        
        // Если на этом уровне были дети, добавляем их в результат
        if (!curChildren.empty()) 
            res.push_back(std::move(curChildren));  
        
        // Меняем очереди местами: следующий уровень становится текущим
        std::swap(cur, next);             
    }
    
    // Вычисляем время выполнения
    timeNs = duration_cast<nanoseconds>(high_resolution_clock::now() - start).count();
    return res;                            // возвращаем результат
}


int main() {
  
    vector<pair<string,string>> rel = {
        {"Адам", "Каин"},
        {"Адам", "Авель"},
        {"Каин", "Енох"},
        {"Енох", "Ирад"},
        {"Ирад", "Мехиаэль"},
        {"Адам", "Сиф"},
        {"Сиф", "Енос"}
    };
    string target = "Адам";   

    long long t1, t2;       
    
    // Запуск со стандартной очередью
    auto r1 = descendants2Q<queue<string>>(rel, target, t1);
    // Запуск со своей очередью
    auto r2 = descendants2Q<MyQueue>(rel, target, t2);

    // Проверяем, что результаты совпадают
    if (r1 == r2) {
        cout << "Потомки " << target << ":\n";
        vector<string> names = {"дети", "внуки", "правнуки", "праправнуки"};
        for (size_t i = 0; i < r1.size(); ++i) {
            cout << names[i] << ": ";
            for (const auto& name : r1[i])
                cout << name << " ";
            cout << "\n";
        }
    }

    // Вывод времени выполнения
    cout << "\nВремя (std::queue): " << t1 << " ns\n";
    cout << "Время (MyQueue):    " << t2 << " ns\n";

    return 0;
}
