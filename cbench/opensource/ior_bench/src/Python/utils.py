#/*****************************************************************************\
#*                                                                             *
#*       Copyright (c) 2003, The Regents of the University of California       *
#*     See the file COPYRIGHT for a complete copyright notice and license.     *
#*                                                                             *
#\*****************************************************************************/
#
# A group of generic utility functions used by IOR-GUI.py
# but not not related to the display itself.
#
# CVS info:
#   $RCSfile: utils.py,v $
#   $Revision: 1.17 $
#   $Date: 2004/07/16 15:01:19 $
#   $Author: loewe $

import string
from tkFileDialog import *
from tkMessageBox import *                                # sets ERROR = 'error'
from Tkinter import *
from definitions import *

################################################################################
# class for creating a scrolling list box                                      #
################################################################################
class scrollListBox:
	def __init__(self, myframe, mylist, myrow, mycol):
		self.mylist = mylist
		# Make a frame to hold the scrolled listbox
		self.frame = Frame(myframe, bg="cornsilk")
		self.frame.grid(row=myrow, column=mycol, sticky=W)
		self.gui_scroll = Scrollbar(self.frame, orient=VERTICAL)
		self.gui_listBox = Listbox(self.frame, bg="#EFEFFF",
					   selectmode=EXTENDED,
					   exportselection=0, width=8,
					   height=7,
					   yscrollcommand=self.gui_scroll.set)
		self.gui_scroll.config(command=self.gui_listBox.yview)
		for item in mylist:
			self.gui_listBox.insert(END, item)    
		self.gui_scroll.pack(side=RIGHT, fill=Y)
		self.gui_listBox.pack(side=LEFT,fill=BOTH)

	def slb_extract(self):
		return extract(self.gui_listBox)


	def slb_select(self, items):
		return setListboxSelections(self.gui_listBox, items)


##########################################################
# method to make a list of selected items from a listbox #
##########################################################
def extract(listbox):
	idx = listbox.curselection()
	items = []
	# Collect all the selected values:
	# try: is because of some problem in older versions of python.
	try:
		idx = map(string.atoi, idx)
	except ValueError: pass

	for k in idx:
		items = items + [listbox.get(k)]
	return items


################################
# method to get name of a file #
################################
def getFileName(which, exists, initDir, extension=""):
	ftitle = 'Get name of %sfile' % which
	if which == '':
		ft1 = "All_Files *" 
	else:
		ft1 = "%s_Files %s" %  (which, which)
	ft2 = "All_Files *"
	if exists == 'exists':
		fileName = askopenfilename(title=ftitle,
					   filetypes=[(ft1), (ft2)],
					   defaultextension=extension,
					   initialdir=initDir)
	else:
		fileName = asksaveasfilename(title=ftitle,
					     filetypes=[(ft1), (ft2)],
					     defaultextension=extension,
					     initialdir=initDir)
	if fileName == "":
		return ""
	else:
		return fileName


###############################################
# method to determine if string is an integer #
###############################################
def isInteger(string):
	success = 0
	for character in range(len(string)):
		try:
			int(string)	# this is a hack for '3g', e.g.
			int(string[character])
			success = 1
		except:
			success = 0
			break
	return success


##############################################
# method to merge two lists into proper list #
##############################################
def mergeLists(list1, list2):
	tempDict = {}
	for s in list1, list2:
		for x in s:
			tempDict[x] = 1
	combinedList = []
	for i in tempDict.keys():
		combinedList.append(i)
	tempDict = {}
	for x in combinedList:
		tempDict[x] = 1
	combinedList = tempDict.keys()
	if len(combinedList) > 0: combinedList.sort()
	return combinedList


#####################################################################
# method to set listbox selections from list of items (not indices) #
#####################################################################
def setListboxSelections(listbox, items):
	listbox.selection_clear(0, END)
	content = listbox.get(0, END)
	s = ""
	for s in items:
		kt = 0
		for t in content:
			if s == t:
				listbox.selection_set(kt)
				break
			else:
				kt = kt + 1
        return listbox


####################################
# method to convert string to list #
####################################
def stringToList(s):
    oldList = string.split(s)
    newList = []
    
    try:
        for token in oldList:
            # pare off ','
            if token[-1:] == ',':
                token = token[:-1]
        
            # multiply by K|M|G suffix
            if token[-1:] == 'K' or token[-1:] == 'k':
                token = str(long(token[:-1])*KIBIBYTE)
            if token[-1:] == 'M' or token[-1:] == 'm':
                token = str(long(token[:-1])*MEBIBYTE)
            if token[-1:] == 'G' or token[-1:] == 'g':
                token = str(long(token[:-1])*GIBIBYTE)
            newList.append(token)
    except:
        newList = ERROR
    return newList


########################################
# method to convert KiB & MiB to Bytes #
########################################
def toBytes(mult, inList):
	outList = []
	for item in inList:
		outList.append(str(mult * string.atoi(item)))
	return outList
