#!/usr/bin/python3
from gi.repository import Gtk, GObject
from graph_tool.all import *
from threading import Thread
import subprocess, time, shutil, os, re, timeit

GObject.threads_init()
openw = 0

def draw(pFile=None, gFile=None, oFile=None, vfontsize=12, vfontfamily = "Ubuntu, Light", outputsize=(1000, 800), outputlocation = None):
	
	if outputlocation is None:
		outputlocation = os.getcwd()
		
	for f in os.listdir(outputlocation):
		if re.search("cagd_phase*", f):
			os.remove(os.path.join(outputlocation, f))
	
	# Creating empty graph and vertex and edge lists
	g = Graph(directed = False)
	vertex_list = []
	edge_list = []


	# Opening Graph file, numnodes are the number of vertices and demand stores the demand vector
	lines = [line.strip() for line in open(gFile)]
	numnodes = int(lines[0])    
	demand = g.new_vertex_property("string")


	# Populating vertices
	for i in range(0, numnodes):
		vertex_list.append(g.add_vertex())


	# Populating edges
	for i in range(1, (len(lines) - 1)):
		ab = lines[i].split()
		a, b = int(ab[0]), int(ab[1])
		if g.edge(b, a) == None:
			edge_list.append(g.add_edge(vertex_list[a], vertex_list[b]))
		

	# Opening	Benchmark file	
	lines = [line.strip() for line in open(pFile)]


	# Populating the demand vector as a Property Map for the graph g
	for i in range(0, numnodes):
		demand[vertex_list[i]] = str(i) +" [" + lines[i+3] +"]"
	

	# Draw the problem graph
	graph_draw(g, output_size = outputsize, nodesfirst = True, vertex_text = demand, vorder = g.vertex_index, vertex_size = 10, vertex_color = "black", vertex_fill_color = "black", vertex_font_size = vfontsize, vertex_font_family = vfontfamily, vertex_text_position = 1, edge_pen_width = 0.5, output = "cagd_phase0000.svg")

	shutil.copy("cagd_phase0000.svg", outputlocation)
	os.remove("cagd_phase0000.svg")



	# Opening the Output file
	lines = [line.strip() for line in open(oFile)]


	# Define new property map for edge widths and frequency assignments
	edge_width = g.new_edge_property("float")
	freq_assign = g.new_vertex_property("string")



	

	# Parse Output file and construct graphs for each phase
	i, phase = 0, 0
	while i < len(lines):

		phase = lines[i].split()[1]
		vlist = []
		i = i + 2
	
		# Reset edge widths to default value
		for a in g.edges():
			edge_width[a] = 0.5
	
		# Populating the frequncy assigned vector as a Property Map for the graph g with initial string consisting of just the node index
		for a in range(0, numnodes):
			freq_assign[vertex_list[a]] = str(a)
		
		# Find vlist, the list of vertices assigned frequencies in current phase, then sort
		while lines[i] != "":
			vertex = int(lines[i].split()[0].split(":")[0])
			vlist.append(vertex)
			flist = lines[i].split()[1:]
			freq_assign[vertex_list[vertex]] +=  " [" + flist[0] + "-" + flist[-1] +"]"
			i = i + 1

		vlist.sort()
	
	
		# For all edges belonging to vertices in vlist, set thickness to high value
		for k in vlist:
			for l in vlist[vlist.index(k)+1:]:
				if g.edge(k, l) != None:
					edge_width[g.edge(k, l)] = 4			
	
	
		# Draw the graph for current phase
		outputfilename = "cagd_phase"+("%04d" % int(phase))+".svg"
		graph_draw(g, nodesfirst = True, vertex_text = freq_assign, vorder = g.vertex_index, vertex_size = 10, vertex_color = "black", vertex_fill_color = "black", vertex_font_size = vfontsize, vertex_font_family = vfontfamily, vertex_text_position = 0, edge_pen_width = edge_width, output_size = outputsize, output = outputfilename)
	
		shutil.copy(outputfilename, outputlocation)
		os.remove(outputfilename)
	
		i = i + 1
		
		
		
		
#####################################################################################################################################################



builder = Gtk.Builder()
builder.add_from_file("GUI.glade")
window = builder.get_object("channelAssigner")

pfcb = builder.get_object("pfilech")
pfcb.set_filename(os.path.join(os.getcwd(), "Files", "Benchmark Problems", "Bench6.txt"))

gfcb = builder.get_object("gfilech")
gfcb.set_filename(os.path.join(os.getcwd(), "Files", "Graph Files", "Graph1.txt"))

ofcb = builder.get_object("ofilech")
ofcb.set_filename(os.path.join(os.getcwd(), "Files", "Output Files", "Bench6 Output"))

class Handler:
	def __init__(self):
		self.pFile = None
		self.gFile = None
		self.oFile = None
		self.ofLoc = None
		self.timed = False
		
	def gtk_main_quit(self, *args):
		
		global openw
		openw -= 1
		
		if openw == -1:
			Gtk.main_quit(*args)
		
	def pfileset(self, widget):
		self.pFile = widget.get_filename()
		
		if self.gFile is not None:
			self.check_inputs()
	
	def gfileset(self, widget):
		self.gFile = widget.get_filename()
		
		if self.pFile is not None:
			self.check_inputs()
			
	def check_inputs(self):		
		with open (self.pFile, "r") as pf:
			p = pf.read()
			    			
		with open (self.gFile, "r") as gf:
    			g = gf.read()
    			    		
		if re.search(r'\b-[\d]+\b', p):
    			self.report_invalid_input(errcode = 1)
    			return False    		
    		
		if re.search(r'\b-[\d]+\b', g):
    			self.report_invalid_input(errcode = 2)
    			return False    	
    		
		if re.search(r'[^\d\s]+', p):
    			self.report_invalid_input(errcode = 3)
    			return False
    		
		if re.search(r'[^\d\s]+', g):
    			self.report_invalid_input(errcode = 4)
    			return False
    			
		pvals = re.findall(r'\b[\d]+\b', p)
		gvals = re.findall(r'\b[\d]+\b', g)
    		
		if len(pvals) == 0:
    			self.report_invalid_input(errcode = 5)
    			return False
    			
		if len(gvals) == 0:
    			self.report_invalid_input(errcode = 6)
    			return False
    		
		vertices = int(gvals[0])
		if len(pvals) != (vertices + 3):
    			self.report_invalid_input(errcode = 7)
    			return False
    		
    		   			
		return True
		
		
	def report_invalid_input(self, errcode=0):

		l = builder.get_object("status")
		
		if errcode == 0:
			l.set_markup("<span foreground=\"red\">Invalid input files</span>")
		elif errcode == 1:
			l.set_markup("<span foreground=\"red\">Invalid problem file, negative values not allowed</span>")
		elif errcode == 2:
			l.set_markup("<span foreground=\"red\">Invalid graph file, negative values not allowed</span>")
		elif errcode == 3:
			l.set_markup("<span foreground=\"red\">Invalid characters in problem file, only numerical data allowed</span>")
		elif errcode == 4:
			l.set_markup("<span foreground=\"red\">Invalid characters in graph file, only numerical data allowed</span>")
		elif errcode == 5:
			l.set_markup("<span foreground=\"red\">Invalid input, problem file is empty</span>")
		elif errcode == 6:
			l.set_markup("<span foreground=\"red\">Invalid input, graph file is empty</span>")
		elif errcode == 7:
			l.set_markup("<span foreground=\"red\">Invalid input, problem parameters do not match graph input</span>")
		
	
	def ofileset(self, widget):
		self.ofLoc = widget.get_filename()
		self.oFile = os.path.join(self.ofLoc, "Output.txt")
		
	def timetogglek(self, widget):
		if widget.get_active():
			self.timed = True
		else:
			self.timed = False
	
	def timetoggle(self, widget, data):
		if widget.get_active():
			self.timed = False
		else:
			self.timed = True
	
	def compute_clicked(self, widget):
		
		b = builder.get_object("btncompute")
		l = builder.get_object("status")
		s = builder.get_object("spinner1")
		
		if self.pFile is None:
			if pfcb.get_filename() is not None:
				self.pfileset(pfcb)
			else:
				l.set_text("Please select a valid .txt file containing problem parameters, or generate one!")
				return
			
		if self.gFile is None:
			if gfcb.get_filename() is not None:
				self.gfileset(gfcb)
			else:
				l.set_text("Please select a valid .txt file describing the problem graph, or generate one!")
				return
			
		if self.oFile is None:
			if ofcb.get_filename() is not None:
				self.ofileset(ofcb)
			else:
				l.set_text("Please select the output location!")
				return
				
		valid = self.check_inputs()
		
		if valid:		
			b.set_sensitive(False)	
			l.set_text("Computing!")
			s.start()
		
			t = Thread(target=self.compute_thread, args = (b, l, s))
			t.start()
			
		
	def compute_thread(self, button, label, spinner):
		
		
		try:
			if self.timed:
				time = min(timeit.Timer("subprocess.check_output([\"./AssignFrequency\", \""+self.pFile+"\", \""+self.gFile+"\", \""+self.oFile+"\"])", setup = "import subprocess").repeat(3, 1000))
				draw(pFile = self.pFile, gFile = self.gFile, oFile = self.oFile, outputlocation = self.ofLoc)
			
				label.set_markup("<span foreground=\"green\">Execution completed! Time: "+str(time)+" milliseconds (averaged over 1000 executions)</span>")
				button.set_sensitive(True)
				spinner.stop()
			else:
				timeit.Timer("subprocess.check_output([\"./AssignFrequency\", \""+self.pFile+"\", \""+self.gFile+"\", \""+self.oFile+"\"])", setup = "import subprocess").timeit(number = 1)
				
				draw(pFile = self.pFile, gFile = self.gFile, oFile = self.oFile, outputlocation = self.ofLoc)
			
				label.set_markup("<span foreground=\"green\">Execution completed!</span>")
				button.set_sensitive(True)
				spinner.stop()
			
		except subprocess.CalledProcessError as cpe:
			label.set_markup("<span foreground=\"red\">Invalid Input! Reason: %s </span>" % cpe.output.replace("\n", " ").decode("utf-8"))
			button.set_sensitive(True)
			spinner.stop()
			
	def gpf(self, widget):
		global openw
		builder = Gtk.Builder()
		builder.add_from_file("GUI.glade")
		
		gpfw = builder.get_object("generatepfile")
		builder.connect_signals(Handler())
		gpfw.show_all()
		openw += 1
		
	def ggf(self, widget):
		global openw
		builder = Gtk.Builder()
		builder.add_from_file("GUI.glade")
		
		ggfw = builder.get_object("generategfile")
		builder.connect_signals(Handler())
		ggfw.show_all()
		openw += 1
		
	def gpfsaveclicked(self, widget):
		
		dialog = Gtk.FileChooserDialog("Save Problem File", widget.get_toplevel(), Gtk.FileChooserAction.SAVE, (Gtk.STOCK_CANCEL, Gtk.ResponseType.CANCEL, Gtk.STOCK_OPEN, Gtk.ResponseType.OK))

		self.add_filters(dialog)
		response = dialog.run()
		
		s0 = int(widget.get_toplevel().get_children()[0].get_children()[0].get_children()[2].get_value())
		s1 = int(widget.get_toplevel().get_children()[0].get_children()[0].get_children()[4].get_value())
		s2 = int(widget.get_toplevel().get_children()[0].get_children()[0].get_children()[6].get_value())
		dvtbuf = widget.get_toplevel().get_children()[0].get_children()[2].get_children()[0].get_buffer()
		
		dv = dvtbuf.get_text(dvtbuf.get_start_iter(), dvtbuf.get_end_iter(), False)
		
		if response == Gtk.ResponseType.OK:
			
			fname = dialog.get_filename()
			with open(fname, "w") as f:
				f.write("%d %d %d\n" % (s0, s1, s2))
				f.write("%s" % (dv, ))				
			
		dialog.destroy()

	def ggfsaveclicked(self, widget):
		
		dialog = Gtk.FileChooserDialog("Save Graph File", widget.get_toplevel(), Gtk.FileChooserAction.SAVE, (Gtk.STOCK_CANCEL, Gtk.ResponseType.CANCEL, Gtk.STOCK_OPEN, Gtk.ResponseType.OK))

		self.add_filters(dialog)
		response = dialog.run()
		
		nodes = int(widget.get_toplevel().get_children()[0].get_children()[0].get_children()[2].get_value())
		etbuf = widget.get_toplevel().get_children()[0].get_children()[2].get_children()[0].get_buffer()
		
		ed = etbuf.get_text(etbuf.get_start_iter(), etbuf.get_end_iter(), False)
		
		if response == Gtk.ResponseType.OK:
			
			fname = dialog.get_filename()
			with open(fname, "w") as f:
				f.write("%d\n" % (nodes))
				f.write("%s" % (ed, ))				
			
		dialog.destroy()
		
	def gpfcancelclicked(self, widget):
		widget.get_toplevel().destroy()
		self.gtk_main_quit()

	def ggfcancelclicked(self, widget):
		widget.get_toplevel().destroy()
		self.gtk_main_quit()
		
	
	def add_filters(self, dialog):
		filter_text = Gtk.FileFilter()
		filter_text.set_name("Text files")
		filter_text.add_mime_type("text/plain")
		dialog.add_filter(filter_text)
			
##########################################################################################################################################



builder.connect_signals(Handler())

window.show_all()
openw += 1
Gtk.main()
