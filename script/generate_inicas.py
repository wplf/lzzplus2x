import os
import random 

PRE_VACANCY = 100

BOX_X = 97
BOX_Y = 99
BOX_Z = 101


vacancy_number = 2 * BOX_X * BOX_Y * BOX_Z * PRE_VACANCY // 10**6

with open('inicas.txt', 'w') as fw:
    fw.write("#id, type, x, y, z, size0, size1, size2, size3, dir\n")
    for i in range(vacancy_number):
        fw.write('\t'.join(map(str, [i+1, -1, random.uniform(0, BOX_X), \
                                    random.uniform(0, BOX_Y), \
                                    random.uniform(0, BOX_Z),
                                    1, 0, 0, 0, -1])))
        fw.write('\n')