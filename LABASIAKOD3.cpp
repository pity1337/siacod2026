#include <iostream>
using namespace std;
// Рекурсивная функция для генерации возрастающих последовательностей
// current - текущая строящаяся последовательность
// n - максимальное число
// m - требуемая длина последовательности
// start - следующее число
void generate(vector<int>& current, int n, int m, int start) {
    if (current.size() == m) {
        for (int num : current) {
            cout << num << " ";
        }
        cout << endl;
        return;
    }
    
    
    for (int i = start; i <= n; i++) {
        current.push_back(i);           
        generate(current, n, m, i + 1);  
        current.pop_back();            
    }
}

int main() {
    int n, m;
    
    cout << "Введите n (максимальное число): ";
    cin >> n;
    cout << "Введите m (длина последовательности, m ≤ n): ";
    cin >> m;
    
    if (m > n) {
        cout << "Ошибка: m должно быть не больше n!" << endl;
        return 1;
    }
    
    cout << "\nВсе возрастающие последовательности длины " << m 
         << " из чисел от 1 до " << n << ":" << endl;
    
    vector<int> current; 
    generate(current, n, m, 1);
    
    return 0;
}
