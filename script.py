import sys
import numpy as np
import matplotlib.pyplot as plt
# from adjustText import adjust_text #extern library to install it conda install -c phlya adjusttext 
from scipy import interpolate 
import toyplot # pip install toyploy
import math

# import ghostscript
import toyplot.png
import toyplot.pdf


def plot_vertexes(x,y,n, edges_plot):
    coordinates=np.transpose(np.vstack((x,y)))
    layout = toyplot.layout.FruchtermanReingold()
    vstyle = {"stroke":toyplot.color.black}
    vlstyle = {"fill":"white"}
    colormap = toyplot.color.LinearMap(toyplot.color.Palette(["white"])) #, "yellow", "red"]))
    canvas, axes, mark = toyplot.graph(edges_plot, #extra_vertices ,
                                       vcoordinates=coordinates, layout=layout,
                                       vcolor=colormap, vsize=30, vstyle=vstyle, width=10000)
    axes.show = True
    axes.aspect = None
    axes.y.show = False
    axes.x.show=False
    
    toyplot.pdf.render(canvas, "nodes.pdf")

def init_edges(f, n, edges):
	line = f.readline()
	while (line.strip() != "NON ZERO VARIABLES"):
	    line = f.readline()
	for i in range(n):
	    line = f.readline()
	    chuncks = line.split(" ")
	    coords = chuncks[0].split("_")
	    edges[i] = [int(str(coords[1])), int(str(coords[2]))]
	edges = edges.astype(int)	

def init_nodes(f, n, x, y):
    for i in range(n):
        line = f.readline()
        x[i] = float(line.split(" ")[0])
        y[i] = float(line.split(" ")[1])
            
def main(toplot):
    f = open(toplot,"r")
    nnodes = int(f.readline())
    xcoord = np.zeros(nnodes)
    ycoord = np.zeros(nnodes)
    edges = np.empty(shape=(nnodes,2), dtype='i4')
    init_nodes(f,nnodes, xcoord, ycoord)
    init_edges(f,nnodes, edges)
    plot_vertexes(xcoord, ycoord, nnodes, edges)
            
    
if __name__ == '__main__':
	main(sys.argv[1])
