import matplotlib.pyplot as plt
import re

def parse_data_file(filename):
    sections = {}
    current_section = None

    with open(filename, 'r') as f:
        for line in f:
            line = line.strip()

            if line.startswith('#-----'):
                current_section = line.replace('#-----', '').strip().split('-----')[0].strip()
                sections[current_section] = {'q': [], 'bler': []}
                continue
            
            match = re.match(r'q\s*=\s*([\d.eE+-]+)\s+bler\s*=\s*([\d.eE+-]+)', line)

            if match and current_section:
                sections[current_section]['q'].append(float(match.group(1)))
                sections[current_section]['bler'].append(float(match.group(2)))
    return sections

data = parse_data_file('BLER-data.txt')

plt.figure(figsize=(10, 6))
colors = ['red', 'orange', 'green', 'blue', 'purple']
for idx, (section, values) in enumerate(data.items()):
    plt.plot(values['q'], values['bler'], 'o-', color=colors[idx], label=f'k = {section} бит', markersize=6)

plt.yscale('log')
plt.xlabel('Параметр канала $q$')
plt.ylabel('Вероятность блочной ошибки, BLER($q$)')
plt.title('Вероятность ошибки на блок в канале c АБГШ для кодов PUCCH format 2')
plt.grid(True, which="both", ls="--")
plt.legend(title='Длина информационного блока')
plt.tight_layout()
plt.show()
