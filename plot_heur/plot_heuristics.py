import sys
import numpy as np
import matplotlib.pyplot as plt
from scipy import interpolate 
import math
import fileinput

if __name__ == '__main__':
	problem = []
	for line in sys.argv[1:len(sys.argv)-1]:
		f = open(line,"r")
		name = f.readline().strip("\n")
		value = []
		time = []
		for i, line in enumerate(f):
			chuncks = line.split(" ")
			value.append(float(chuncks[0]))
			time.append(float(chuncks[1].strip("\n")))
		plt.plot(time, value, label=name)
		
	for i, prob in enumerate(problem):
		problem[i] = prob.strip("\n")

	plt.xlabel('Time (s)')
	plt.ylabel('Value of the objective functions')
	y = np.array(value)
	m =  math.floor(y.min())
	M = math.ceil(y.max())
	plt.yticks(np.arange(m, M, (M-m)/10))
	plt.legend()
	plt.grid(True, linewidth=0.25)
	plt.title((sys.argv[len(sys.argv)-1]))
	plt.savefig('plot_heur/plot_heur.pdf', format='pdf', bbox_inches='tight')
	plt.show()
	



