F = 10
P = 22

for i in range(F):
    l = [0x00] + [i == j / 3 and 0xFF or 0x80 for j in range(3 * P)] + [0x00]
    print '    { %s },' % ', '.join(hex(j) for j in l)
