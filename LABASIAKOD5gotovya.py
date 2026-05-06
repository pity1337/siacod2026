import tkinter as tk
from tkinter import filedialog, simpledialog, messagebox, colorchooser
import networkx as nx 
import matplotlib.pyplot as plt
import random 
import heapq 

G = nx.DiGraph() 
highlight_color = "red"  


def get_valid_node(title="Выбор вершины"):
    if not G.nodes():  # Проверяем, не пуст ли граф
        messagebox.showerror("Ошибка", "Граф пуст!") 
        return None 
    nodes = list(G.nodes())
    start = simpledialog.askstring(title, f"Доступные вершины: {nodes}")
    if start in G: return start 
    try:
        val = int(start) 
        if val in G: return val
    except: pass 
    messagebox.showerror("Ошибка", f"Вершина '{start}' не найдена!")
    return None 

def choose_color():
    global highlight_color
    color = colorchooser.askcolor(title="Выберите цвет для выделения")[1]
    if color: 
        highlight_color = color

def load_graph():
    """Функция для чтения графа из внешнего файла .graphml"""
    global G 
    file_path = filedialog.askopenfilename(filetypes=[("GraphML", "*.graphml"), ("Все файлы", "*.*")])
    if file_path:
        try:
            G = nx.read_graphml(file_path)
            if not G.is_directed(): G = G.to_directed()
            for u in G.nodes(): 
                if 'value' not in G.nodes[u]: G.nodes[u]['value'] = random.randint(1, 100) 
            for u, v in G.edges(): 
                if 'weight' not in G[u][v]: G[u][v]['weight'] = random.randint(1, 10)
            draw_graph(G) 
            messagebox.showinfo("Успех", "Граф успешно загружен из GraphML!")
        except Exception as e:
            messagebox.showerror("Ошибка загрузки", f"Не удалось прочитать файл:\n{e}")


def draw_graph(graph, highlight=None, filter_condition=None):
    plt.clf()
    fig, ax = plt.subplots(num=1)  # Создаем область рисования
    if filter_condition: 
        try:
            nodes_to_show = [n for n, attr in graph.nodes(data=True) if eval(f"{attr.get('value', 0)} {filter_condition}")]  # Фильтруем узлы
            display_graph = graph.subgraph(nodes_to_show)  # Создаем временный граф только из подходящих узлов
        except Exception as e:  
            messagebox.showerror("Ошибка фильтра", f"Некорректное условие: {e}") 
            display_graph = graph  # Отменяем фильтрацию
    else: display_graph = graph  # Иначе берем текущий граф целиком
    if not display_graph.nodes(): return  # Если после фильтрации не осталось узлов — выходим
    pos = nx.circular_layout(display_graph)  # Расставляем узлы по кругу
    node_colors = [highlight_color if n in (highlight or []) else "skyblue" for n in display_graph.nodes()]  # Красим узлы
    nx.draw_networkx_nodes(display_graph, pos, node_color=node_colors, node_size=600, ax=ax)  # Рисуем кружочки узлов
    labels = {n: f"{n}\n(v:{attr.get('value', 0)})" for n, attr in display_graph.nodes(data=True)}  # Подписи для узлов
    nx.draw_networkx_labels(display_graph, pos, labels=labels, font_size=9, ax=ax)  # Рисуем текст имен и значений
    nx.draw_networkx_edges(display_graph, pos, ax=ax, arrowstyle='-|>', arrowsize=20, connectionstyle="arc3,rad=0.2", edge_color="gray", width=1.5)  # Рисуем дуги
    edge_labels = nx.get_edge_attributes(display_graph, "weight")  # Извлекаем веса для подписей связей[cite: 3]
    nx.draw_networkx_edge_labels(display_graph, pos, edge_labels=edge_labels, label_pos=0.3, font_size=8, ax=ax, rotate=False, connectionstyle="arc3,rad=0.2")  # Рисуем веса на дугах
    plt.axis('off') 
    plt.show()


def run_bfs():
    """Запуск поиска в ширину"""
    node = get_valid_node("BFS")
    if node is not None:
        result = my_bfs(node)  # Считаем порядок обхода
        messagebox.showinfo("Результат BFS", f"Порядок: {result}") 
        draw_graph(G, highlight=result)  # Вывод визуально

def run_dfs():
    """Запуск поиска в глубину"""
    node = get_valid_node("DFS")
    if node is not None:
        result = my_dfs(node)  # Считаем порядок обхода
        messagebox.showinfo("Результат DFS", f"Порядок: {result}")  # Вывод текстом
        draw_graph(G, highlight=result)  # Вывод визуально

def my_bfs(start):
    visited, queue = [], [start]  # Посещенные узлы и очередь ожидания
    while queue:  # Пока в очереди кто-то есть
        node = queue.pop(0) 
        if node not in visited: 
            visited.append(node) 
            queue.extend([n for n in G.neighbors(node) if n not in visited])  # Добавляем всех соседей в конец очереди
    return visited

def my_dfs(node, visited=None):
    if visited is None: visited = []
    visited.append(node)  
    for n in G.neighbors(node):  # Смотрим на соседей текущего узла
        if n not in visited: my_dfs(n, visited)  # Если соседа не видели — идем в нег
    return visited 

def my_dijkstra(start):
    dist = {n: float("inf") for n in G.nodes()}
    dist[start] = 0  
    pq = [(0, start)]  # Очередь с приоритетом (расстояние, вершина)
    while pq: 
        d, node = heapq.heappop(pq)  # Берем самый короткий из найденных путей
        if d > dist[node]: continue  # Если уже нашли путь короче — игнорируем
        for neigh in G.neighbors(node):
            weight = G[node][neigh].get("weight", 1)  # Берем цену перехода
            if d + weight < dist[neigh]:  # Если через текущий узел быстрее
                dist[neigh] = d + weight  # Записываем новый рекорд
                heapq.heappush(pq, (dist[neigh], neigh)) 
    return dist  

def my_floyd():
    nodes = list(G.nodes())
    dist = {u: {v: (0 if u == v else float("inf")) for v in nodes} for u in nodes}  
    for u, v in G.edges(): dist[u][v] = G[u][v].get("weight", 1) 
    for k in nodes: 
        for i in nodes:
            for j in nodes: 
                dist[i][j] = min(dist[i][j], dist[i][k] + dist[k][j])  # Если через K короче — обновляем путь
    return dist  

def generate_graph():
    global G
    graph_type = simpledialog.askstring("Тип", "Вид: complete , cycle, tree, random ")
    n = simpledialog.askinteger("Вершины", "Количество вершин:") 
    if not n: return  
    if graph_type == "complete": G = nx.complete_graph(n, create_using=nx.DiGraph)
    elif graph_type == "cycle": G = nx.cycle_graph(n, create_using=nx.DiGraph)  
    elif graph_type == "tree": G = nx.full_rary_tree(2, n, create_using=nx.DiGraph)
    else: G = nx.erdos_renyi_graph(n, 0.4, directed=True)  
    for u in G.nodes(): G.nodes[u]["value"] = random.randint(1, 100)  # Даем узлам случайные данные
    for u, v in G.edges(): G[u][v]["weight"] = random.randint(1, 10)  # Даем связям случайные веса
    draw_graph(G)

root = tk.Tk()  
root.title("gh2026")  
frame = tk.Frame(root)
frame.pack(padx=20, pady=20)  

tk.Label(frame, text="УПРАВЛЕНИЕ ГРАФОМ", font=("Arial", 10, "bold")).pack()
tk.Button(frame, text="Сгенерировать структуру", command=generate_graph, bg="#e8f5e9", width=35).pack(pady=2)
tk.Button(frame, text="Загрузить .graphml", command=load_graph, bg="#e3f2fd", width=35).pack(pady=2)
tk.Button(frame, text="Выбрать цвет выделения", command=choose_color, width=35).pack(pady=2)

tk.Label(frame, text="\nАЛГОРИТМЫ", font=("Arial", 10, "bold")).pack()
tk.Button(frame, text="BFS (Обход + Окно)", command=run_bfs, width=35).pack(pady=2)
tk.Button(frame, text="DFS (Обход + Окно)", command=run_dfs, width=35).pack(pady=2)
tk.Button(frame, text="Дейкстра (Кратчайший путь)", command=lambda: messagebox.showinfo("Результат", f"Дистанции: {my_dijkstra(get_valid_node())}"), width=35).pack(pady=2)
tk.Button(frame, text="Флойд (Матрица в консоль)", command=lambda: print("Матрица путей:\n", my_floyd()), width=35).pack(pady=2)

tk.Label(frame, text="\nВИЗУАЛИЗАЦИЯ", font=("Arial", 10, "bold")).pack()
tk.Button(frame, text="Обновить/Показать всё", command=lambda: draw_graph(G), width=35).pack(pady=2)
tk.Button(frame, text="Фильтр по условию (v)", command=lambda: draw_graph(G, filter_condition=simpledialog.askstring("Фильтр", "Условие (напр. > 50):")), width=35).pack(pady=2)

root.mainloop()
