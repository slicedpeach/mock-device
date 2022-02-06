import os
import random
import threading
path = "/dev/mycode"
fd = os.open(path, os.O_RDWR)
r = 'r'
r = str.encode(r)
os.write(fd,r)
def writing():
	for i in range(1000000):
		line = f'e,{random.randint(1,99)},0,1'
		line  = str.encode(line)
		os.write(fd,line)
p1 = threading.Thread(target = writing)
p2 = threading.Thread(target = writing)
p1.start()
p2.start()

p1.join()
p2.join()
result = os.read(fd,2000)

print(result.decode())

os.close(fd)
