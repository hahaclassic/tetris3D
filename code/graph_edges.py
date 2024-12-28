import matplotlib.pyplot as plt
import numpy as np

# Данные для графика
edges = [2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30, 32]
render_time = [642, 707, 798, 921, 1084, 1280, 1545, 1883, 2321, 2832, 3410, 4069, 4822, 5646, 6560, 7569]

# Построение графика
plt.figure(figsize=(10, 6))

# График
# plt.scatter(edges, render_time, color='red', label='Данные, полученные в ходе исследования')
# coeffs = np.polyfit(edges, render_time, 4)  # Кубическая аппроксимация
# quad = np.poly1d(coeffs)
# x = np.linspace(min(edges), max(edges), 500)
# plt.plot(x, quad(x), linestyle='-', color='blue', alpha=0.7, label=f'Аппроксимирующий полином: y={coeffs[0]:.4f}x^4 + {coeffs[1]:.4f}x^3 + {coeffs[2]:.4f}x^2 + {coeffs[3]:.4f}x + {coeffs[4]:.4f}')
# print(f'y={coeffs[0]:.4f}x^4 + {coeffs[1]:.4f}x^3 + {coeffs[2]:.4f}x^2 + {coeffs[3]:.4f}x + {coeffs[4]:.4f}')
plt.scatter(edges, render_time, color='red', label='Данные, полученные в ходе исследования')
coeffs = np.polyfit(edges, render_time, 2)
quad = np.poly1d(coeffs)
x = np.linspace(min(edges), max(edges), 500)
plt.plot(x, quad(x), linestyle='-', color='blue', alpha=0.7, label=f'Аппроксимирующий полином: y={coeffs[0]:.2f}x^2 + {coeffs[1]:.2f}x + {coeffs[2]:.2f}')

# Настройки графика
plt.title('Зависимость времени отрисовки кадра от количества аппроксимирующих рёбер')
plt.xlabel('Количество рёбер, шт')
plt.ylabel('Время отрисовки, мс')
plt.grid(True, linestyle='--', alpha=0.7)
plt.legend()

# Более частые засечки на осях
plt.xticks(np.arange(0, 35, 2))
plt.yticks(np.arange(0, 8001, 500))

plt.tight_layout()

# Сохранение графика и отображение
plt.savefig('render_time_edges_graph.png')
plt.show()
