import random


for i in range(1000):
    with open(f'./text/numbers_{i}.txt', 'w') as f:
        for j in range(1000):
            nums = round(random.uniform(0.1, 100000.0), 4)
            f.write(f'{nums}\n')