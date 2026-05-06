#include <iostream>
#include <vector>
#include <stack>
#include <chrono>

using namespace std;
using namespace chrono;

template<typename T>
class MyStack {
private:
    T* data;          // указатель на массив элементов
    size_t capacity;  // текущая ёмкость
    size_t size;      // текущее количество элементов

    // Увеличение ёмкости в 2 раза
    void resize() {
        size_t newCap = capacity * 2;
        T* newData = new T[newCap];
        for (size_t i = 0; i < size; ++i)
            newData[i] = std::move(data[i]);   // перемещаем старые элементы
        delete[] data;
        data = newData;
        capacity = newCap;
    }

public:
    // Конструктор с начальной ёмкостью
    MyStack(size_t initialCap = 1024) : data(new T[initialCap]), capacity(initialCap), size(0) {}

    // Деструктор
    ~MyStack() { delete[] data; }

    // Запрещаем копирование
    MyStack(const MyStack&) = delete;
    MyStack& operator=(const MyStack&) = delete;

    // Разрешаем перемещение
    MyStack(MyStack&& other) noexcept
        : data(other.data), capacity(other.capacity), size(other.size) {
        other.data = nullptr;
        other.capacity = 0;
        other.size = 0;
    }

    // Добавление элемента 
    void push(T&& value) {
        if (size == capacity) resize();
        data[size] = std::move(value);
        ++size;
    }

    // Удаление верхнего элемента
    void pop() {
        if (size > 0) --size;
    }

   
	void reserve(size_t newCap) {
	    if (newCap > capacity) {
	        T* newData = new T[newCap];
	        for (size_t i = 0; i < size; ++i)
	            newData[i] = std::move(data[i]);
	        delete[] data;
	        data = newData;
	        capacity = newCap;
	    }
	}

    // Доступ к верхнему элементу
    T& top() {
        return data[size - 1];
    }

    // Проверка на пустоту
    bool empty() const {
        return size == 0;
    }
};

template<typename Stack>
void processSequence(const vector<int>& seq, Stack& st) {
    if (seq.empty()) return;

    vector<int> current;
    current.push_back(seq[0]);

    for (size_t i = 1; i < seq.size(); ++i) {
        if (seq[i] > seq[i - 1]) {
            current.push_back(seq[i]);           // продолжаем серию
        } else {
            st.push(std::move(current));         // серия закончилась – в стек
            current.clear();
            current.push_back(seq[i]);            // начинаем новую
        }
    }
    st.push(std::move(current));                  // последняя серия
}
//добавление
template<typename Stack>
void printStack(Stack& st) {
    while (!st.empty()) {
        vector<int> series = std::move(st.top());
        st.pop();
        for (int x : series) cout << x << " ";
        cout << endl;
    }
}

// ГЛАВНАЯ ФУНКЦИЯ
int main() {
    // Ввод данных от пользователя
    cout << "Введите количество чисел: ";
    int n;
    cin >> n;
    vector<int> userSeq;
    cout << "Введите " << n << " целых чисел: ";
    for (int i = 0; i < n; ++i) {
        int x;
        cin >> x;
        userSeq.push_back(x);
    }

    cout << "\nВозрастающие серии в обратном порядке (std::stack):\n";
    {
        stack<vector<int>> stdStack;
        auto start = high_resolution_clock::now();
        processSequence(userSeq, stdStack);
        auto end = high_resolution_clock::now();
        printStack(stdStack);
        auto timeStd = duration_cast<nanoseconds>(end - start).count();
        cout << "Время std::stack: " << timeStd << " ns\n";
    }
	MyStack<vector<int>> myStack;
	myStack.reserve(n); 
	processSequence(userSeq, myStack);
    // Обработка со своим стеком
    cout << "\nВозрастающие серии в обратном порядке (MyStack):\n";
    {
        MyStack<vector<int>> myStack;
        auto start = high_resolution_clock::now();
        processSequence(userSeq, myStack);
        auto end = high_resolution_clock::now();
        printStack(myStack);
        auto timeMy = duration_cast<nanoseconds>(end - start).count();
        cout << "Время MyStack: " << timeMy << " ns\n";
    }

    return 0;
}
