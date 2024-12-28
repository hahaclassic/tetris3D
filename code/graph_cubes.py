import matplotlib.pyplot as plt
import numpy as np

# Данные для графика без теней
cubes_no_shadows = [10, 60, 110, 160, 210, 260, 310, 360, 410, 460, 510, 560, 610, 660, 710, 760, 810, 860, 910, 960]
render_time_no_shadows = [11, 43, 77, 103, 134, 161, 184, 214, 235, 266, 307, 317, 343, 365, 412, 423, 452, 496, 552, 591]

# Данные для графика с тенями
cubes_with_shadows = [10, 60, 110, 160, 210, 260, 310, 360, 410, 460, 510, 560, 610, 660, 710, 760, 810, 860, 910, 960]
render_time_with_shadows = [19, 72, 130, 162, 196, 227, 259, 292, 324, 388, 455, 485, 514, 544, 578, 622, 622, 656, 703, 729]

# Построение графика
plt.figure(figsize=(10, 6))

# График без теней
plt.scatter(cubes_no_shadows, render_time_no_shadows, color='blue', label='Данные, полученные в ходе исследования (отрисовка без теней)')
coeffs_no_shadows = np.polyfit(cubes_no_shadows, render_time_no_shadows, 1)
line_no_shadows = np.poly1d(coeffs_no_shadows)
x_no_shadows = np.linspace(min(cubes_no_shadows), max(cubes_no_shadows), 500)
plt.plot(x_no_shadows, line_no_shadows(x_no_shadows), linestyle='-', color='blue', alpha=0.7, label=f'Аппроксимирующий полином: y={coeffs_no_shadows[0]:.2f}x + {coeffs_no_shadows[1]:.2f}')

# График с тенями
plt.scatter(cubes_with_shadows, render_time_with_shadows, color='red', label='Данные, полученные в ходе исследования (отрисовка с тенями)')
coeffs_with_shadows = np.polyfit(cubes_with_shadows, render_time_with_shadows, 1)
line_with_shadows = np.poly1d(coeffs_with_shadows)
x_with_shadows = np.linspace(min(cubes_with_shadows), max(cubes_with_shadows), 500)
plt.plot(x_with_shadows, line_with_shadows(x_with_shadows), linestyle='-', color='red', alpha=0.7, label=f'Аппроксимирующий полином: y={coeffs_with_shadows[0]:.2f}x + {coeffs_with_shadows[1]:.2f}')

# Настройки графика
plt.title('Зависимость времени отрисовки кадра от количества объектов на сцене')
plt.xlabel('Количество объектов (кубов), шт')
plt.ylabel('Время отрисовки, мс')
plt.grid(True, linestyle='--', alpha=0.7)
plt.legend()

# Более частые засечки на осях
plt.xticks(np.arange(0, 1001, 50))
plt.yticks(np.arange(0, 801, 50))

plt.tight_layout()

# Сохранение графика и отображение
plt.savefig('render_time_graph.png')
plt.show()

# Расчет отношения времен и процентного ускорения
speedup = [render_time_with_shadows[i] / render_time_no_shadows[i] for i in range(len(render_time_no_shadows))]
acceleration_percent = [100 * (1 - render_time_no_shadows[i] / render_time_with_shadows[i]) for i in range(len(render_time_no_shadows))]

# Построение графика отношения времен
plt.figure()
plt.plot(cubes_no_shadows, speedup)
plt.xlabel('Количество объектов')
plt.ylabel('Отношение времен рендеринга')
plt.title('Ускорение при отключении теней')
plt.grid(True)
plt.show()

# import numpy as np

# # Данные для времени отрисовки без теней
# render_time_no_shadows = [11, 43, 77, 103, 134, 161, 184, 214, 235, 266, 307, 317, 343, 365, 412, 423, 452, 496, 552, 591]

# # Данные для времени отрисовки с тенями
# render_time_with_shadows = [19, 72, 130, 162, 196, 227, 259, 292, 324, 388, 455, 485, 514, 544, 578, 622, 622, 656, 703, 729]

# # Расчёт процентного изменения для каждой пары значений
# percent_differences = [(with_shadows - no_shadows) / no_shadows * 100 
#                        for with_shadows, no_shadows in zip(render_time_with_shadows, render_time_no_shadows)]

# # Среднее процентное изменение
# average_percent_difference = np.mean(percent_differences)

# print(f"Среднее процентное увеличение времени отрисовки с тенями: {average_percent_difference:.2f}%")
