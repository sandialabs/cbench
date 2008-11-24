#!/usr/local/bin/python
# GUI for IOR
#
#/*****************************************************************************\
#*                                                                             *
#*       Copyright (c) 2003, The Regents of the University of California       *
#*     See the file COPYRIGHT for a complete copyright notice and license.     *
#*                                                                             *
#\*****************************************************************************/
#
# CVS info:
#   $RCSfile: IOR-GUI.py,v $
#   $Revision: 1.49 $
#   $Date: 2006/07/12 00:04:47 $
#   $Author: loewe $


# In building the GUI interface we found that several variable
# names were required for most of the quantities of interest.
# An attempt was made to use the following naming convention 
# for these variables to enhance readability.
#
# Variable x from Values Class :   self.vals.x
# Selected value in Values Class:  self.vals.sel_x
# Corresponding StringVariable:    self.strvar_x
# GUI display variable:            self.gui_x
#
# v2t.py  2/25 ==> This version has two scrollable lists side by side.
#         3/11 ==> Save & Load options working for top half of original layout.
 
from Tkinter import *
from tkFileDialog import *
from tkMessageBox import *                                # sets ERROR = 'error'
from ScrolledText import *
from Values import *
from definitions import *
from utils import *
import os.path
import string
import sys
import time

################################################################################
# class to hold window configuration and necessary methods                     #
################################################################################
class ConfigureWindow:

    ########################################
    # initialization of values and widgets #
    ########################################
    def __init__(self, master):
        self.debug = 0
        self.vals = Values()
        self.frame = Frame(master, bg=bgColor)       

	# create items in frame
        self.createMenuBar()
        self.createTitle()
        self.createIOLayerEntries()
        self.createPlatformParameters()
        self.createSystemResources()
        self.createFileLocations()
        self.createSizeLists()
        self.createTestParameters()
        self.createButtons()

        self.frame.pack()


    #############################
    # method to create menu bar #
    #############################
    def createMenuBar(self):
        menubar = Menu(root)

	filemenu = Menu(menubar, tearoff=0)
	# Note: additional option 'accelerator="?"' may be added
	filemenu.add_command(label="Submit", command=self.submitJobs)
	filemenu.add_command(label="Review Tests", command=self.reviewTests)
	filemenu.add_command(label="Save Config", command=self.saveSettings)
	filemenu.add_command(label="Load Config", command=self.loadSettings)
	filemenu.add_command(label="Reset", command=self.reset)
	filemenu.add_separator()
	filemenu.add_command(label="Exit", command=self.quit)
	menubar.add_cascade(label="File", menu=filemenu)

	helpmenu = Menu(menubar, tearoff=0)
	helpmenu.add_command(label="Help", command=self.displayHelp)
	helpmenu.add_command(label="About", command=self.createAboutBox)
	menubar.add_cascade(label="Help", menu=helpmenu)

	root.config(menu=menubar)


    ##########################################
    # method to create title and information #
    ##########################################
    def createTitle(self):
        self.title = Frame(self.frame, bg=bgColor)
        Label(self.title, bg=bgColor, text=" IOR ", bd=3, relief=RAISED,
              font=titleFont).grid(row=0)
        self.title.grid(row=0, column=0, columnspan=4, padx=10, pady=10)


    ########################################################
    # method to create I/O Layer entries from Values class #
    ########################################################
    def createIOLayerEntries(self):
        self.IOLayer = Frame(self.frame, bg=bgColor)
        Label(self.IOLayer, bg=bgColor, font=labelFont,
	      text="I/O LAYER").pack(side=TOP)

        self.gui_listBox_IOLayer = \
	    Listbox(self.IOLayer, bg=fieldColor, selectmode=SINGLE,
		    exportselection=0, width=10, height=4)
        for item in self.vals.IOLayer:
            self.gui_listBox_IOLayer.insert(END, item)
        self.gui_listBox_IOLayer.select_set(0)
        self.vals.sel_IOLayer = self.vals.IOLayer[0]
        self.gui_listBox_IOLayer.pack(side=TOP)
        self.IOLayer.grid(row=1, column=0, sticky=N, padx=10, pady=0)


    ###############################################
    # method to create platform parameter entries #
    ###############################################
    def createPlatformParameters(self):
        self.Platform = Frame(self.frame, bg=bgColor)
        Label(self.Platform, bg=bgColor, font=labelFont,
	      text=" PLATFORM ").pack(side=TOP)

        self.strvar_platform = StringVar()
	platforms = self.vals.platformDict.keys()
	platforms.sort()
	del platforms[platforms.index('OTHER')]
	platforms = [self.Platform, self.strvar_platform] + \
		    platforms + ['OTHER']
        self.gui_platform = apply(OptionMenu, platforms)

        # Changes here must also be made in Values.py
        self.strvar_platform.set(self.vals.default_platform)
        self.gui_platform.pack(side=TOP)
        self.gui_platformOptionsButton = \
	    Button(self.Platform, activebackground=activeColor, bg=buttonColor,
		   text="PLATFORM\nSETTINGS", fg=buttonTextColor,
		   command=self.changePlatformParameters)
        self.gui_platformOptionsButton.pack(side=TOP)
        
        self.Platform.grid(row=1, column=1, sticky=N, padx=10, pady=0)

    #############################################
    # method to create system resources entries #
    #############################################
    def createSystemResources(self):
        self.Tasks = Frame(self.frame, bg=bgColor)
        Label(self.Tasks, bg=bgColor, font=labelFont,
	      text="SYSTEM RESOURCES").grid(row=0, column=0, columnspan=2,
					    sticky=W)

        # Set up Number of Nodes and Tasks Per Node in 2 separate columns
	# inside the same grided frame.

        # Left Column for node selection:

        Label(self.Tasks, bg=bgColor, text="Nodes").grid(row=1, column=0,
							 sticky=W)
        
        self.strvar_nodes = StringVar()
        self.strvar_nodes.set(str(self.vals.nodes))

        self.gui_nodes = Entry(self.Tasks, bg=fieldColor, width=8,
			       textvariable=self.strvar_nodes)        
        self.gui_nodes.grid(row=2, column=0, sticky=W)

        # make a frame to hold the scrolled listbox
        self.sysResNodes = Frame(self.Tasks, bg=bgColor)
        self.sysResNodes.grid(row=3, column=0, sticky=W)
        self.gui_scrollNodesList = Scrollbar(self.sysResNodes, orient=VERTICAL)
        self.gui_listBox_nodesList = \
	    Listbox(self.sysResNodes, bg=fieldColor, selectmode=EXTENDED,
		    exportselection=0, width=8, height=4,
		    yscrollcommand=self.gui_scrollNodesList.set)
        self.gui_scrollNodesList.config(
	    command=self.gui_listBox_nodesList.yview)
        for item in self.vals.nodesList:
            self.gui_listBox_nodesList.insert(END, item)
        self.gui_scrollNodesList.pack(side=RIGHT, fill=Y)
        self.gui_listBox_nodesList.pack(side=LEFT, fill=BOTH)

        # 2nd Column for tasks per node selection:
        Label(self.Tasks, bg=bgColor, text="Tasks Per Node").grid(row=1,
            column=1, sticky=W)
        self.strvar_tasksPerNode = StringVar()
        self.strvar_tasksPerNode.set(str(self.vals.tasksPerNode))
        self.tasksPerNode = Entry(self.Tasks, bg=fieldColor, width=8,
                                  textvariable=self.strvar_tasksPerNode)        
        self.tasksPerNode.grid(row=2, column=1, sticky=W)

        self.sysResTPN = Frame(self.Tasks, bg=bgColor)
        self.sysResTPN.grid(row=3, column=1, sticky=W)
        self.gui_scrollTPNList = Scrollbar(self.sysResTPN, orient=VERTICAL)
        self.gui_listBox_tasksPerNodeList = \
	    Listbox(self.sysResTPN, bg=fieldColor, selectmode=EXTENDED,
		    exportselection=0, width=8, height=4,
		    yscrollcommand=self.gui_scrollTPNList.set)
        self.gui_scrollTPNList.config(
	    command=self.gui_listBox_tasksPerNodeList.yview)
        for item in self.vals.tasksPerNodeList:
            self.gui_listBox_tasksPerNodeList.insert(END, item)
        self.gui_scrollTPNList.pack(side=RIGHT, fill=Y)
        self.gui_listBox_tasksPerNodeList.pack(side=LEFT, fill=BOTH)

        self.Tasks.grid(row=1, column=2, sticky=N, padx=10, pady=0)

        # separate frame for Machine Group, Job Start Command, JOB Time,
        # Dependency, etc.
        self.SysRes = Frame(self.frame, bg=bgColor)
        Label(self.SysRes, bg=bgColor,
              text="Machine Group").grid(row=1, column=0, sticky=W)
        Label(self.SysRes, bg=bgColor,
              text="Job Start").grid(row=2, column=0, sticky=W)
        Label(self.SysRes, bg=bgColor,
              text="Max Clock Time").grid(row=3, column=0, sticky=W)
        Label(self.SysRes, bg=bgColor,
              text="Job Dependency").grid(row=4, column=0, sticky=W)
        Label(self.SysRes, bg=bgColor,
              text="Earliest Submit Time").grid(row=5, column=0, sticky=W)
        Label(self.SysRes, bg=bgColor,
              text="PSUB Options").grid(row=6, column=0, sticky=W)
        
        self.strvar_machGroup = StringVar()
        Label(self.SysRes, bg=bgColor, text=" ").grid(row=0, column=0, sticky=W)
	self.vals.machGroups.sort()
	machGroups = [self.SysRes, self.strvar_machGroup,
		      self.vals.default_machGroup] + self.vals.machGroups
        self.gui_machGroup = apply(OptionMenu, machGroups)
        self.strvar_machGroup.set(self.vals.default_machGroup)
        self.gui_machGroup.grid(row=1, column=1, sticky=W)

        self.strvar_jobStart = StringVar()
	self.vals.jobStarts.sort()
	jobStarts = [self.SysRes, self.strvar_jobStart,
		      self.vals.default_jobStart] + self.vals.jobStarts
        self.gui_jobStart = apply(OptionMenu, jobStarts)
        self.strvar_jobStart.set(self.vals.default_jobStart)
        self.gui_jobStart.grid(row=2, column=1, sticky=W)        

        self.strvar_maxClockTime = StringVar()
        self.strvar_maxClockTime.set(str(self.vals.maxClockTime))
        self.maxClockTime = Entry(self.SysRes, bg=fieldColor, width=6,
				  textvariable=self.strvar_maxClockTime)        
        self.maxClockTime.grid(row=3, column=1, sticky=W)

        self.strvar_jobDepend = StringVar()
        self.strvar_jobDepend.set(str(self.vals.jobDepend))
        self.jobDepend = Entry(self.SysRes, bg=fieldColor, width=6,
			       textvariable=self.strvar_jobDepend)        
        self.jobDepend.grid(row=4, column=1, sticky=W)

        self.strvar_startTime = StringVar()
        self.strvar_startTime.set(str(self.vals.startTime))
        self.gui_startTime = Entry(self.SysRes, bg=fieldColor, width=15,
				   textvariable=self.strvar_startTime)        
        self.gui_startTime.grid(row=5, column=1, sticky=W)
        
        self.strvar_psubOptions = StringVar()
        self.strvar_psubOptions.set(str(self.vals.psubOptions))
        self.gui_psubOptions = Entry(self.SysRes, bg=fieldColor, width=15,
				     textvariable=self.strvar_psubOptions)
        self.gui_psubOptions.grid(row=6, column=1, sticky=W)
        
        self.SysRes.grid(row=1, column=3, sticky=N, padx=10, pady=0)
        

    ######################################
    # method to create file path entries #
    ######################################
    def createFileLocations(self):
        self.strvar_scriptFile = StringVar()
        self.strvar_scriptFile.set(str(self.vals.scriptFile))
        self.strvar_codeFile = StringVar()
        self.strvar_codeFile.set(str(self.vals.codeFile))
        self.strvar_testFile = StringVar()
        self.strvar_testFile.set(str(self.vals.testFile))
        self.strvar_hintFile = StringVar()
        self.strvar_hintFile.set(str(self.vals.hintFile))

        self.fileLocation = Frame(self.frame, bg=bgColor)

        Label(self.fileLocation, bg=bgColor, font=labelFont,
	      text="FILE LOCATIONS").grid(row=0, column=0, sticky=W)
        Label(self.fileLocation, bg=bgColor,
	      text="Script & Results").grid(row=1, column=0, sticky=W)
        self.gui_scriptButton = \
	    Button(self.fileLocation, activebackground=activeColor,
		   bg=buttonColor, text="Browse", fg=buttonTextColor,
                   command=self.getScriptFileName)
        self.gui_scriptButton.grid(row=1, column=1, sticky=W)
        self.gui_scriptFile = Entry(self.fileLocation, bg=fieldColor, width=67,
                                textvariable=self.strvar_scriptFile)
        self.gui_scriptFile.grid(row=1, column=2)
        
        Label(self.fileLocation, bg=bgColor,
	      text="Executable Code").grid(row=2, column=0, sticky=W)
        self.gui_codeButton = \
	    Button(self.fileLocation, activebackground=activeColor,
                   bg=buttonColor, text="Browse", fg=buttonTextColor,
                   command=self.getCodeFileName)
        self.gui_codeButton.grid(row=2, column=1, sticky=W)
        self.gui_codeFile = Entry(self.fileLocation, bg=fieldColor, width=67,
                                  textvariable=self.strvar_codeFile)
        self.gui_codeFile.grid(row=2, column=2)

        Label(self.fileLocation, bg=bgColor,
	      text="Temp Data File").grid(row=3, column=0, sticky=W)
        self.gui_testButton = \
	    Button(self.fileLocation, activebackground=activeColor,
                   bg=buttonColor, text="Browse", fg=buttonTextColor,
                   command=self.gettestFileName)
        self.gui_testButton.grid(row=3, column=1, sticky=W)
        self.gui_testFile = Entry(self.fileLocation, bg=fieldColor, width=67,
                                  textvariable=self.strvar_testFile)
        self.gui_testFile.grid(row=3, column=2)
        
        Label(self.fileLocation, bg=bgColor,
	      text="Hint File").grid(row=4, column=0, sticky=W)
        self.gui_testButton = \
	    Button(self.fileLocation, activebackground=activeColor,
                   bg=buttonColor, text="Browse", fg=buttonTextColor,
                   command=self.getHintFileName)
        self.gui_testButton.grid(row=4, column=1, sticky=W)
        self.gui_hintFile = Entry(self.fileLocation, bg=fieldColor, width=67,
                                  textvariable=self.strvar_hintFile)
        self.gui_hintFile.grid(row=4, column=2)
        
        self.fileLocation.grid(row=2, column=0, sticky=W,
			       columnspan=4, padx=10, pady=0)


    #############################################################
    # method to create block & transfer lists from Values class #
    #############################################################
    def createSizeLists(self):
        # IOR block/transfer lists
        self.TestParameters = Frame(self.frame, bg=bgColor)
        self.Lists = Frame(self.TestParameters, bg=bgColor)

        Label(self.Lists, bg=bgColor, font=labelFont,
	      text="TEST PARAMETERS").grid(row=0, column=0,
					   columnspan=2, sticky=W)

        Label(self.Lists, bg=bgColor,
	    text="Transfer Size").grid(row=1, column=0, sticky=W)
        self.strvar_transferSize = StringVar()
        self.strvar_transferSize.set(str(self.vals.transferSize))
        self.gui_transferSize = Entry(self.Lists, bg=fieldColor, width=11,
                                      textvariable=self.strvar_transferSize)
        self.gui_transferSize.insert(END, self.vals.transferSize)
        self.gui_transferSize.grid(row=1, column=1)

        Label(self.Lists, bg=bgColor, text="KiB").grid(row=3, column=0)
        Label(self.Lists, bg=bgColor, text="MiB").grid(row=3, column=1)

        Label(self.Lists, bg=bgColor, text=" ").grid(row=3, column=2, rowspan=2)
        
        self.slb_KiB_TransferSize = \
	    scrollListBox(self.Lists, self.vals.KiB_transferSizeList, 4, 0)
        self.slb_MiB_TransferSize = \
	    scrollListBox(self.Lists, self.vals.MiB_transferSizeList, 4, 1)

        Label(self.Lists, bg=bgColor, text=" ").grid(row=1, column=2)

        Label(self.Lists, bg=bgColor, text="Block Size").grid(row=1, column=3)
        self.strvar_blockSize = StringVar()
        self.strvar_blockSize.set(str(self.vals.blockSize))
        self.gui_blockSize = \
	    Entry(self.Lists, bg=fieldColor, width=11,
		  textvariable=self.strvar_blockSize)        
        self.gui_blockSize.grid(row=1, column=4)

        Label(self.Lists, bg=bgColor, font=smallFont,
	      text="(Use CTRL key with mouse for multi-select or deselect.)"
	     ).grid(row=2, column=0, columnspan=5)

        Label(self.Lists, bg=bgColor, text="KiB").grid(row=3, column=3)
        Label(self.Lists, bg=bgColor, text="MiB").grid(row=3, column=4)

        # Scrolled ListBoxes for KiB & MiB blocksizes
        self.slb_KiB_BlockSize = \
	    scrollListBox(self.Lists, self.vals.KiB_blockSizeList, 4, 3)
        self.slb_MiB_BlockSize = \
	    scrollListBox(self.Lists, self.vals.MiB_blockSizeList, 4, 4)

        self.Lists.grid(row=0, column=0, padx=5, pady=0)


    ####################################
    # method to create test parameters #
    ####################################
    def createTestParameters(self):

	checkBoxWidth = 22
        self.Settings = Frame(self.TestParameters, bg=bgColor)
        Label(self.Settings, bg=bgColor, font=smallFont,
              text=" ").grid(row=0, column=0, sticky=W)
        Label(self.Settings, bg=bgColor, font=smallFont,
              text="Segments/Datasets").grid(row=1, columnspan=2,
                                             column=0, sticky=W)
        self.strvar_segmentCount = StringVar()
        self.strvar_segmentCount.set(str(self.vals.segmentCount))
        self.segmentCount = Entry(self.Settings, bg=fieldColor, width=4,
                                  textvariable=self.strvar_segmentCount)        
        self.segmentCount.grid(row=1, column=2, sticky=E)

        Label(self.Settings, bg=bgColor, font=smallFont,
              text="Test Repetitions").grid(row=2, columnspan=2,
                                            column=0, sticky=W)
        self.strvar_testReps = StringVar()
        self.strvar_testReps.set(str(self.vals.testReps))
        self.testReps = Entry(self.Settings, bg=fieldColor, width=4,
                              textvariable=self.strvar_testReps)        
        self.testReps.grid(row=2, column=2, sticky=E)

        Label(self.Settings, bg=bgColor, font=smallFont,
              text="Number of Tasks").grid(row=3, columnspan=2,
                                                 column=0, sticky=W)
        self.strvar_numTasks = StringVar()
        self.strvar_numTasks.set(str(self.vals.numTasks))
        self.numTasks = Entry(self.Settings, bg=fieldColor, width=4,
                              textvariable=self.strvar_numTasks)        
        self.numTasks.grid(row=3, column=2, sticky=E)

        Label(self.Settings, bg=bgColor, font=smallFont,
              text="Intertest Delay (Sec)").grid(row=4, columnspan=2,
                                                 column=0, sticky=W)
        self.strvar_interTestDelay = StringVar()
        self.strvar_interTestDelay.set(str(self.vals.interTestDelay))
        self.gui_interTestDelay = Entry(self.Settings, bg=fieldColor, width=4,
                     			textvariable=self.strvar_interTestDelay)
        self.gui_interTestDelay.grid(row=4, column=2, sticky=E)
              
        Label(self.Settings, bg=bgColor, font=smallFont,
              text="Max Time (0=off)").grid(row=5, columnspan=2,
					     column=0, sticky=W)
        self.strvar_maxTimeDuration = StringVar()
        self.strvar_maxTimeDuration.set(str(self.vals.maxTimeDuration))
        self.gui_maxTimeDuration = Entry(self.Settings, bg=fieldColor, width=4,
                     		 textvariable=self.strvar_maxTimeDuration)
        self.gui_maxTimeDuration.grid(row=5, column=2, sticky=E)
              
        Label(self.Settings, bg=bgColor, font=smallFont,
              text="Outlier Threshold (Sec)").grid(row=6, columnspan=2,
					     column=0, sticky=W)
        self.strvar_outlierThreshold = StringVar()
        self.strvar_outlierThreshold.set(str(self.vals.outlierThreshold))
        self.gui_outlierThreshold = Entry(self.Settings, bg=fieldColor, width=4,
                     		    textvariable=self.strvar_outlierThreshold)
        self.gui_outlierThreshold.grid(row=6, column=2, sticky=E)
              
        Label(self.Settings, bg=bgColor, font=smallFont,
              text="Verbosity").grid(row=7, columnspan=2, column=0, sticky=W)
        self.strvar_verbose = StringVar()
        self.strvar_verbose.set(str(self.vals.verbose))
        self.gui_verbose = Entry(self.Settings, bg=fieldColor, width=4,
                     		 textvariable=self.strvar_verbose)
        self.gui_verbose.grid(row=7, column=2, sticky=E)
              
        Label(self.Settings, bg=bgColor, font=smallFont,
              text="Set Time Stamp Signature").grid(row=8, columnspan=2,
              column=0, sticky=W)
        self.strvar_setTimeStampSignature = StringVar()
        self.strvar_setTimeStampSignature.set(str(
              self.vals.setTimeStampSignature))
        self.gui_setTimeStampSignature = Entry(self.Settings, bg=fieldColor,
	      width=4, textvariable=self.strvar_setTimeStampSignature)
        self.gui_setTimeStampSignature.grid(row=8, column=2, sticky=E)
              
        Label(self.Settings, bg=bgColor, font=smallFont,
              text="Options").grid(row=9, column=0, sticky=W)
        self.strvar_options = StringVar()
        self.strvar_options.set(str(self.vals.options))
        self.gui_options = Entry(self.Settings, bg=fieldColor, width=14,
                     		 textvariable=self.strvar_options)
        self.gui_options.grid(row=9, columnspan=2, column=1, sticky=E)
              
        self.intvar_keepFile = IntVar()
        self.intvar_keepFile.set(self.vals.keepFile)
        self.gui_keepFile = Checkbutton(self.Settings, bg=fieldColor,
					  variable=self.intvar_keepFile,
					  selectcolor=activeColor,
					  text="Keep Test File",
					  width=checkBoxWidth, font=smallFont,
					  anchor=W)
        self.gui_keepFile.grid(row=10, column=0, columnspan=3, sticky=W)
        
        self.intvar_keepFileWithError = IntVar()
        self.intvar_keepFileWithError.set(self.vals.keepFileWithError)
        self.gui_keepFileWithError = Checkbutton(self.Settings, bg=fieldColor,
					variable=self.intvar_keepFileWithError,
					selectcolor=activeColor,
					text="Keep Test File With Error",
					width=checkBoxWidth, font=smallFont,
					anchor=W)
        self.gui_keepFileWithError.grid(row=11, column=0, columnspan=3, sticky=W)
        
        self.intvar_preallocate = IntVar()
        self.intvar_preallocate.set(self.vals.preallocate)
        self.gui_preallocate = Checkbutton(self.Settings, bg=fieldColor,
					   variable=self.intvar_preallocate,
					   selectcolor=activeColor,
					   text="Preallocate Test File",
					   width=checkBoxWidth, font=smallFont,
					   anchor=W)
        self.gui_preallocate.grid(row=12, column=0, columnspan=3, sticky=W)
        
        self.intvar_filePerProc = IntVar()
        self.intvar_filePerProc.set(self.vals.filePerProc)
        self.gui_filePerProc = Checkbutton(self.Settings, bg=fieldColor,
					   variable=self.intvar_filePerProc,
					   selectcolor=activeColor,
					   text="One File Per Process",
					   width=checkBoxWidth, font=smallFont,
					   anchor=W)
        self.gui_filePerProc.grid(row=13, column=0, columnspan=3, sticky=W)

        self.intvar_showHints = IntVar()
        self.intvar_showHints.set(self.vals.showHints)
        self.gui_showHints = Checkbutton(self.Settings, bg=fieldColor,
                        		 variable=self.intvar_showHints,
					 selectcolor=activeColor,
					 text="Show Hints", anchor=W,
					 width=checkBoxWidth, font=smallFont)
        self.gui_showHints.grid(row=14, column=0, columnspan=3, sticky=W)
        
        self.intvar_reorderTasks = IntVar()
        self.intvar_reorderTasks.set(self.vals.reorderTasks)
        self.gui_reorderTasks = Checkbutton(self.Settings, bg=fieldColor,
                                            variable=self.intvar_reorderTasks,
                                            selectcolor=activeColor,
                                            text="Reorder Tasks", anchor=W,
                                            width=checkBoxWidth, font=smallFont)
        self.gui_reorderTasks.grid(row=15, column=0, columnspan=3, sticky=W)
        
        self.intvar_singleXferAttempt = IntVar()
        self.intvar_singleXferAttempt.set(self.vals.singleXferAttempt)
        self.gui_singleXferAttempt = \
	    Checkbutton(self.Settings, bg=fieldColor,
			variable=self.intvar_singleXferAttempt,
			selectcolor=activeColor, text="Single POSIX Transfer",
			anchor=W, width=checkBoxWidth, font=smallFont)
        self.gui_singleXferAttempt.grid(row=16, column=0, columnspan=3,
                                        sticky=W)

        self.intvar_useExistingTestFile = IntVar()
        self.intvar_useExistingTestFile.set(self.vals.useExistingTestFile)
        self.gui_useExistingTestFile = \
            Checkbutton(self.Settings, bg=fieldColor,
                        variable=self.intvar_useExistingTestFile,
                        selectcolor=activeColor,
                        text="Use Existing Test File", anchor=W,
			width=checkBoxWidth, font=smallFont)
        self.gui_useExistingTestFile.grid(row=17, column=0, columnspan=3,
                                          sticky=W)

        self.intvar_noFill = IntVar()
        self.intvar_noFill.set(self.vals.noFill)
        self.gui_noFill = Checkbutton(self.Settings, bg=fieldColor,
                                      variable=self.intvar_noFill,
                                      selectcolor=activeColor, text="No Fill",
                                      anchor=W, width=checkBoxWidth,
                                      font=smallFont)
        self.gui_noFill.grid(row=18, column=0, columnspan=3, sticky=W)

        Label(self.Settings, bg=bgColor, text=" ").grid(row=0, column=2)

        self.intvar_readFile = IntVar()
        self.intvar_readFile.set(self.vals.readFile)
        self.gui_readFile = Checkbutton(self.Settings, bg=fieldColor,
                 			variable=self.intvar_readFile,
					selectcolor=activeColor,
					text="Read File", anchor=W,
					width=checkBoxWidth, font=smallFont)
        self.gui_readFile.grid(row=1, column=3, sticky=W)

        self.intvar_writeFile = IntVar()
        self.intvar_writeFile.set(self.vals.writeFile)
        self.gui_writeFile = Checkbutton(self.Settings, bg=fieldColor,
                 			 variable=self.intvar_writeFile,
					 selectcolor=activeColor,
					 text="Write File", anchor=W,
					 width=checkBoxWidth, font=smallFont)
        self.gui_writeFile.grid(row=2, column=3, sticky=W)
        
        self.intvar_checkRead = IntVar()
        self.intvar_checkRead.set(self.vals.checkRead)
        self.gui_checkRead = Checkbutton(self.Settings, bg=fieldColor,
                 			 variable=self.intvar_checkRead,
					 selectcolor=activeColor,
					 text="Check Read", anchor=W,
					 width=checkBoxWidth, font=smallFont)
        self.gui_checkRead.grid(row=3, column=3, sticky=W)

        self.intvar_checkWrite = IntVar()
        self.intvar_checkWrite.set(self.vals.checkWrite)
        self.gui_checkWrite = Checkbutton(self.Settings, bg=fieldColor,
                 			  variable=self.intvar_checkWrite,
					  selectcolor=activeColor,
					  text="Check Write", anchor=W,
					  width=checkBoxWidth, font=smallFont)
        self.gui_checkWrite.grid(row=4, column=3, sticky=W)
        
        self.intvar_collective = IntVar()
        self.intvar_collective.set(self.vals.collective)
        self.gui_collective = Checkbutton(self.Settings, bg=fieldColor,
                 			  variable=self.intvar_collective,
					  selectcolor=activeColor,
					  text="Collective", anchor=W,
					  width=checkBoxWidth, font=smallFont)
        self.gui_collective.grid(row=5, column=3, sticky=W)
        
        self.intvar_useFileView = IntVar()
        self.intvar_useFileView.set(self.vals.useFileView)
        self.gui_useFileView = Checkbutton(self.Settings, bg=fieldColor,
                 			   variable=self.intvar_useFileView,
					   selectcolor=activeColor,
					   text="Use File View", anchor=W,
					   width=checkBoxWidth, font=smallFont)
        self.gui_useFileView.grid(row=6, column=3, sticky=W)
        
        self.intvar_useSharedFilePointer = IntVar()
        self.intvar_useSharedFilePointer.set(self.vals.useSharedFilePointer)
        self.gui_useSharedFilePointer = \
	    Checkbutton(self.Settings, bg=fieldColor,
			variable=self.intvar_useSharedFilePointer,
			selectcolor=activeColor, anchor=W,
			text="Use Shared File Pointer",
			width=checkBoxWidth, font=smallFont, state=DISABLED)
        self.gui_useSharedFilePointer.grid(row=7, column=3, sticky=W)

        self.intvar_useStridedDatatype = IntVar()
        self.intvar_useStridedDatatype.set(self.vals.useStridedDatatype)
        self.gui_useStridedDatatype = \
	    Checkbutton(self.Settings, bg=fieldColor,
			variable=self.intvar_useStridedDatatype,
			selectcolor=activeColor, anchor=W,
			text="Use Strided Datatype",
			width=checkBoxWidth, font=smallFont, state=DISABLED)
        self.gui_useStridedDatatype.grid(row=8, column=3, sticky=W)
        
        self.intvar_useIndividualDataSets = IntVar()
        self.intvar_useIndividualDataSets.set(self.vals.useIndividualDataSets)
        self.gui_useIndividualDataSets = \
	    Checkbutton(self.Settings, bg=fieldColor,
			variable=self.intvar_useIndividualDataSets,
			selectcolor=activeColor, anchor=W,
			text="Use Individual Datasets",
			width=checkBoxWidth, font=smallFont, state=DISABLED)
        self.gui_useIndividualDataSets.grid(row=9, column=3, sticky=W)
        
        self.intvar_useO_DIRECT = IntVar()
        self.intvar_useO_DIRECT.set(self.vals.useO_DIRECT)
        self.gui_useO_DIRECT = Checkbutton(self.Settings, bg=fieldColor,
				   	   variable=self.intvar_useO_DIRECT,
					   selectcolor=activeColor, anchor=W,
				   	   text="Use O_DIRECT",
					   width=checkBoxWidth, font=smallFont)
        self.gui_useO_DIRECT.grid(row=10, column=3, sticky=W)

        self.intvar_showHelp = IntVar()
        self.intvar_showHelp.set(self.vals.showHelp)
        self.gui_showHelp = Checkbutton(self.Settings, bg=fieldColor,
                        		variable=self.intvar_showHelp,
					selectcolor=activeColor,
					text="Show Help", anchor=W,
					width=checkBoxWidth, font=smallFont)
        self.gui_showHelp.grid(row=11, column=3, sticky=W)
        
        self.intvar_quitOnError = IntVar()
        self.intvar_quitOnError.set(self.vals.quitOnError)
        self.gui_quitOnError = Checkbutton(self.Settings, bg=fieldColor,
                                           variable=self.intvar_quitOnError,
					   selectcolor=activeColor,
					   text="Quit On Error", anchor=W,
					   width=checkBoxWidth, font=smallFont)
        self.gui_quitOnError.grid(row=12, column=3, sticky=W)

        self.intvar_fsync = IntVar()
        self.intvar_fsync.set(self.vals.fsync)
        self.gui_fsync = Checkbutton(self.Settings, bg=fieldColor,
                                           variable=self.intvar_fsync,
					   selectcolor=activeColor,
					   text="Use fsync", anchor=W,
					   width=checkBoxWidth, font=smallFont)
        self.gui_fsync.grid(row=13, column=3, sticky=W)
        
        self.intvar_uniqueDir = IntVar()
        self.intvar_uniqueDir.set(self.vals.uniqueDir)
        self.gui_uniqueDir = Checkbutton(self.Settings, bg=fieldColor,
                                           variable=self.intvar_uniqueDir,
					   selectcolor=activeColor,
					   text="Use Unique Directory",
					   anchor=W, width=checkBoxWidth,
					   font=smallFont)
        self.gui_uniqueDir.grid(row=14, column=3, sticky=W)

        self.intvar_storeFileOffset = IntVar()
        self.intvar_storeFileOffset.set(self.vals.storeFileOffset)
        self.gui_storeFileOffset = Checkbutton(self.Settings, bg=fieldColor,
                                      variable=self.intvar_storeFileOffset,
                                      selectcolor=activeColor,
				      text="Store File Offset",
                                      anchor=W, width=checkBoxWidth,
                                      font=smallFont)
        self.gui_storeFileOffset.grid(row=15, column=3, sticky=W)
        
        self.intvar_multiFile = IntVar()
        self.intvar_multiFile.set(self.vals.multiFile)
        self.gui_multiFile = Checkbutton(self.Settings, bg=fieldColor,
                                      variable=self.intvar_multiFile,
                                      selectcolor=activeColor,
				      text="Multiple Files",
                                      anchor=W, width=checkBoxWidth,
                                      font=smallFont)
        self.gui_multiFile.grid(row=16, column=3, sticky=W)

        self.intvar_intraTestBarriers = IntVar()
        self.intvar_intraTestBarriers.set(self.vals.intraTestBarriers)
        self.gui_intraTestBarriers = Checkbutton(self.Settings, bg=fieldColor,
                                      variable=self.intvar_intraTestBarriers,
                                      selectcolor=activeColor,
				      text="Intra Test Barriers",
                                      anchor=W, width=checkBoxWidth,
                                      font=smallFont)
        self.gui_intraTestBarriers.grid(row=17, column=3, columnspan=3,
					sticky=W)
        
        self.Settings.grid(row=0, column=1, padx=10, pady=5)
        self.TestParameters.grid(row=3, column=0, columnspan=4, padx=5, pady=5)


    #########################################################
    # method to display buttons for submitting and quitting #
    #########################################################
    def createButtons(self):
        self.buttonBox = Frame(self.frame, bg=bgColor)
        self.runButton = Button(self.buttonBox, activebackground=activeColor,
				bg=buttonColor, text="SUBMIT",
				fg=buttonTextColor, command=self.submitJobs)
        self.runButton.grid(row=0, column=0, sticky=W)
        self.reviewTests = Button(self.buttonBox, activebackground=activeColor,
				  bg=buttonColor, text="REVIEW TESTS",
				  fg=buttonTextColor, command=self.reviewTests)
        self.reviewTests.grid(row=0, column=1, sticky=W)
        self.saveButton = Button(self.buttonBox, activebackground=activeColor,
				 bg=buttonColor, text="SAVE CONFIG",
				 fg=buttonTextColor, command=self.saveSettings)
        self.saveButton.grid(row=0, column=2, sticky=W)
        self.loadButton = Button(self.buttonBox, activebackground=activeColor,
				 bg=buttonColor, text="LOAD CONFIG",
				 fg=buttonTextColor, command=self.loadSettings)
        self.loadButton.grid(row=0, column=3, sticky=W)
        self.resetButton = Button(self.buttonBox, activebackground=activeColor,
				  bg=buttonColor, text="RESET",
				  fg=buttonTextColor, command=self.reset)
        self.resetButton.grid(row=0, column=4, sticky=W)
        self.quitButton = Button(self.buttonBox, activebackground=activeColor,
				 bg=buttonColor, text="EXIT",
				 fg=buttonTextColor, command=self.quit)
        self.quitButton.grid(row=0, column=5, sticky=W)
        self.buttonBox.grid(row=4, column=0, columnspan=4, padx=10, pady=5)


    ####################################
    # methods to submit or review jobs #
    ####################################
    def submitJobs(self):
	self.jobs(preview=FALSE)

    def reviewTests(self):
	self.jobs(preview=TRUE)


    ##################
    # method to quit #
    ##################
    def quit(self):
        answer = askokcancel('Verify exit', 'Do you really want to quit?')
        if answer:
            self.frame.quit()


    ################################
    # method to load an .IOR file #
    ################################
    def loadSettings(self):
        fileName = getFileName('.IORgui', 'exists', self.vals.loadConfigDir,
			       extension='.IORgui')
        if fileName == "":
            return
        else:
            tempVals = self.vals.loadSettingsFromFile(fileName)
	    if (tempVals != ERROR):
                self.vals = tempVals
                tmp = os.path.split(fileName)
	        # preserve chosen directory for next use
                self.vals.loadConfigDir = tmp[0]
		self.reloadSelections(default=FALSE)


    ########################################################################
    # method to get all the selected values and put them into self.vals    #
    ########################################################################
    def getSelectedValues(self):
        idx = self.gui_listBox_IOLayer.curselection()
        IOIndex = string.atoi(idx[0])
        self.gui_listBox_IOLayer.select_set(IOIndex)
        self.vals.sel_IOLayer = self.vals.IOLayer[IOIndex]
        self.vals.platform = self.strvar_platform.get()
        self.vals.nodes = self.strvar_nodes.get()
        self.vals.sel_NodesList = extract(self.gui_listBox_nodesList)
        self.vals.tasksPerNode = self.strvar_tasksPerNode.get()
        self.vals.sel_TasksPerNodeList = \
	    extract(self.gui_listBox_tasksPerNodeList)
        self.vals.machGroup = self.strvar_machGroup.get()
        self.vals.jobStart = self.strvar_jobStart.get()
        self.vals.maxClockTime = self.strvar_maxClockTime.get()
        self.vals.jobDepend = self.strvar_jobDepend.get()
        self.vals.startTime = self.strvar_startTime.get()
        self.vals.psubOptions = self.strvar_psubOptions.get()
                
        self.vals.scriptFile = self.strvar_scriptFile.get()
        self.vals.codeFile = self.strvar_codeFile.get()
        self.vals.testFile = self.strvar_testFile.get()
        self.vals.hintFile = self.strvar_hintFile.get()

        self.vals.segmentCount = self.strvar_segmentCount.get()
        self.vals.testReps = self.strvar_testReps.get()
        self.vals.numTasks = self.strvar_numTasks.get()
        self.vals.interTestDelay = self.strvar_interTestDelay.get()
        self.vals.verbose = self.strvar_verbose.get()
        self.vals.setTimeStampSignature = \
	    self.strvar_setTimeStampSignature.get()
        self.vals.maxTimeDuration = self.strvar_maxTimeDuration.get()
        self.vals.outlierThreshold = self.strvar_outlierThreshold.get()
        self.vals.options = self.strvar_options.get()
        self.vals.filePerProc = self.intvar_filePerProc.get()
        self.vals.keepFile = self.intvar_keepFile.get()
        self.vals.keepFileWithError = self.intvar_keepFileWithError.get()
        self.vals.readFile = self.intvar_readFile.get()        
        self.vals.writeFile = self.intvar_writeFile.get()        
        self.vals.checkRead = self.intvar_checkRead.get()        
        self.vals.checkWrite = self.intvar_checkWrite.get()        
        self.vals.preallocate = self.intvar_preallocate.get()        
        self.vals.showHints = self.intvar_showHints.get()        
        self.vals.reorderTasks = self.intvar_reorderTasks.get()        
        self.vals.showHelp = self.intvar_showHelp.get()        
        self.vals.useExistingTestFile = self.intvar_useExistingTestFile.get()
        self.vals.noFill = self.intvar_noFill.get()
        self.vals.intraTestBarriers = self.intvar_intraTestBarriers.get()
        self.vals.storeFileOffset = self.intvar_storeFileOffset.get()
        self.vals.multiFile = self.intvar_multiFile.get()
        self.vals.quitOnError = self.intvar_quitOnError.get()
        self.vals.fsync = self.intvar_fsync.get()
        self.vals.uniqueDir = self.intvar_uniqueDir.get()
        self.vals.useO_DIRECT = self.intvar_useO_DIRECT.get()
        self.vals.singleXferAttempt = self.intvar_singleXferAttempt.get()
        self.vals.collective = self.intvar_collective.get()        
        self.vals.useFileView = self.intvar_useFileView.get()        
        self.vals.useSharedFilePointer = self.intvar_useSharedFilePointer.get()
        self.vals.useStridedDatatype = self.intvar_useStridedDatatype.get()
        self.vals.useIndividualDataSets = \
	    self.intvar_useIndividualDataSets.get()
        
        self.vals.blockSize = self.strvar_blockSize.get()
        self.vals.transferSize = self.strvar_transferSize.get()
        self.vals.sel_KiB_BlockSizeList = self.slb_KiB_BlockSize.slb_extract()
        self.vals.sel_MiB_BlockSizeList = self.slb_MiB_BlockSize.slb_extract()
        self.vals.sel_KiB_TransferSizeList = \
	    self.slb_KiB_TransferSize.slb_extract()
        self.vals.sel_MiB_TransferSizeList = \
	    self.slb_MiB_TransferSize.slb_extract()
        return


    ######################################################################
    # method to save an .IOR file with all values and current selections #
    ######################################################################
    def saveSettings(self):
        # Get all the selected values and put them into self.vals
        self.getSelectedValues()
        fileName = getFileName('.IORgui', 'not-exist', self.vals.saveConfigDir,
                               extension='.IORgui')
        if fileName == "":
            return
        else:
            tmp = os.path.split(fileName)
	    # preserve chosen directory for next use
            self.vals.saveConfigDir = tmp[0]
            self.vals.saveSettingsToFile(fileName)


    #######################################################################
    # method to reset selections for all of GUI after loading from a file #
    #######################################################################
    def reloadSelections(self, default):
        setListboxSelections(self.gui_listBox_IOLayer, [self.vals.sel_IOLayer])
	if default == TRUE:
            self.strvar_platform.set(self.vals.default_platform)
	else:
            self.strvar_platform.set(self.vals.platform)
        self.strvar_nodes.set(str(self.vals.nodes))
        setListboxSelections(self.gui_listBox_nodesList,
			     self.vals.sel_NodesList)
        self.strvar_tasksPerNode.set(str(self.vals.tasksPerNode))
        setListboxSelections(self.gui_listBox_tasksPerNodeList,
                             self.vals.sel_TasksPerNodeList)
        if default == TRUE:
	    self.strvar_machGroup.set(self.vals.default_machGroup)
        else:
	    self.strvar_machGroup.set(self.vals.machGroup)
        if default == TRUE:
	    self.strvar_jobStart.set(self.vals.default_jobStart)
        else:
	    self.strvar_jobStart.set(self.vals.jobStart)
        self.strvar_maxClockTime.set(str(self.vals.maxClockTime))
        self.strvar_jobDepend.set(str(self.vals.jobDepend))
        self.strvar_startTime.set(str(self.vals.startTime))
        self.strvar_psubOptions.set(str(self.vals.psubOptions))

        self.strvar_scriptFile.set(str(self.vals.scriptFile))
        self.strvar_codeFile.set(str(self.vals.codeFile))
        self.strvar_testFile.set(str(self.vals.testFile))
        self.strvar_hintFile.set(str(self.vals.hintFile))

        self.strvar_segmentCount.set(str(self.vals.segmentCount))
        self.strvar_testReps.set(str(self.vals.testReps))
        self.strvar_numTasks.set(str(self.vals.numTasks))
        self.strvar_interTestDelay.set(str(self.vals.interTestDelay))
        self.strvar_setTimeStampSignature.set(str(
	    self.vals.setTimeStampSignature))
        self.strvar_maxTimeDuration.set(str(self.vals.maxTimeDuration))
        self.strvar_outlierThreshold.set(str(self.vals.outlierThreshold))
        self.strvar_options.set(str(self.vals.options))
        self.intvar_filePerProc.set(self.vals.filePerProc)
        self.intvar_keepFile.set(self.vals.keepFile)
        self.intvar_keepFileWithError.set(self.vals.keepFileWithError)
        self.intvar_readFile.set(self.vals.readFile)
        self.intvar_writeFile.set(self.vals.writeFile)
        self.intvar_checkRead.set(self.vals.checkRead)
        self.intvar_checkWrite.set(self.vals.checkWrite)
        self.intvar_preallocate.set(self.vals.preallocate)
        self.intvar_showHints.set(self.vals.showHints)
        self.intvar_reorderTasks.set(self.vals.reorderTasks)
        self.intvar_showHelp.set(self.vals.showHelp)
        self.intvar_singleXferAttempt.set(self.vals.singleXferAttempt)
        self.intvar_collective.set(self.vals.collective)
        self.intvar_useFileView.set(self.vals.useFileView)
        self.intvar_useSharedFilePointer.set(self.vals.useSharedFilePointer)
        self.intvar_useStridedDatatype.set(self.vals.useStridedDatatype)
        self.intvar_useIndividualDataSets.set(self.vals.useIndividualDataSets)
        self.intvar_useExistingTestFile.set(self.vals.useExistingTestFile)
        self.intvar_noFill.set(self.vals.noFill)
        self.intvar_intraTestBarriers.set(self.vals.intraTestBarriers)
        self.intvar_storeFileOffset.set(self.vals.storeFileOffset)
        self.intvar_multiFile.set(self.vals.multiFile)
        self.intvar_quitOnError.set(self.vals.quitOnError)
        self.intvar_fsync.set(self.vals.fsync)
        self.intvar_uniqueDir.set(self.vals.uniqueDir)
        self.intvar_useO_DIRECT.set(self.vals.useO_DIRECT)
        
        self.strvar_blockSize.set(str(self.vals.blockSize))
        self.strvar_transferSize.set(str(self.vals.transferSize))
        self.slb_KiB_BlockSize.slb_select(self.vals.sel_KiB_BlockSizeList)
        self.slb_MiB_BlockSize.slb_select(self.vals.sel_MiB_BlockSizeList)
        self.slb_KiB_TransferSize.slb_select(self.vals.sel_KiB_TransferSizeList)
        self.slb_MiB_TransferSize.slb_select(self.vals.sel_MiB_TransferSizeList)
        

    ##############################
    # method to create About Box #
    ##############################
    def createAboutBox(self):
        aboutBox = Toplevel()
        aboutBox.title('IOR')
        Label(aboutBox, text='IOR version 2.0\n\n'
            + '(C) Copyright Regents of The University of California and\n'
            + 'Lawrence Livermore National Laboratory 1999-2002\n'
            + 'All rights reserved.\n\n'
            + 'For more information, contact:\n'
	    + 'Scalable I/O Project\nPhone: 4-6975\n'
	    + 'Email: tmclarty@llnl.gov').pack()
        Button(aboutBox, text="OK", command=aboutBox.destroy).pack()
        aboutBox.focus_set()
        aboutBox.grab_set()
        aboutBox.wait_window()


    ######################################
    # method to display help information #
    ######################################
    def displayHelp(self):
        helpBox = Toplevel()
        helpBox.title('IOR Help')
    	textField = ScrolledText(helpBox)
	helpFile = open(self.vals.helpFile, 'r')
	helpInformation = helpFile.read()
	helpFile.close()
        textField.insert(At(0,0), helpInformation)
        textField.pack(side=TOP)
        textField.config(state=DISABLED)
        Button(helpBox, text="OK", command=helpBox.destroy).pack()
        helpBox.focus_set()
        helpBox.grab_set()
        helpBox.wait_window()


    #######################
    # method to reset GUI #
    #######################
    def reset(self):
        self.vals = Values()
        self.reloadSelections(default=TRUE)


    ########################################
    # method to change platform parameters #
    ########################################
    def changePlatformParameters(self):
        self.platformBox = Toplevel()
        self.platformBox.title('Change Platform Parameters')
        self.vals.platform = self.strvar_platform.get()
        Label(self.platformBox, text='Current settings for %s platform' %
              self.vals.platform ).pack()
        self.platformParms = Text(self.platformBox, height=10, width=60,
                                  wrap=WORD, bg=fieldColor)
        #parms.insert(END, self.vals.sel_platformText)
        self.platformParms.insert(END,
	    self.vals.platformDict[self.vals.platform])
        self.platformParms.pack(padx=5, pady=5)
        Label(self.platformBox,
	      text='CHANGE OR ENTER ANY COMMANDS TO RUN BEFORE JOB').pack()
        Button(self.platformBox, text="OK",
	       command=self.updatePlatformParameters).pack()
        self.platformBox.focus_set()
        self.platformBox.grab_set()
        self.platformBox.wait_window()


    ########################################
    # method to change platform parameters #
    ########################################
    def updatePlatformParameters(self):
        self.vals.platformDict[self.vals.platform] = \
	    self.platformParms.get('1.0', END)
        self.platformBox.destroy()


    ####################################
    # method to change test parameters #
    ####################################
    def changeTestOptions(self):
        self.changeTestOptionsBox = Toplevel()
        self.changeTestOptionsBox.title('Change Test Options')
        Label(self.changeTestOptionsBox, bg=bgColor,
              text="Read Only").grid(row=0, column=0, sticky=W)
        self.importantParam = Entry(self.changeTestOptionsBox,
				    bg=fieldColor, width=25)
        self.importantParam.insert(END, self.vals.importantParamEntry)
        self.importantParam.grid(row=0, column=1)
        Button(self.changeTestOptionsBox, text="OK",
               command=self.changeTestOptionsBox.destroy).grid(row=1, sticky=W)
        self.changeTestOptionsBox.focus_set()
        self.changeTestOptionsBox.grab_set()
        self.changeTestOptionsBox.wait_window()


    ######################################
    # method to submit a single PSUB job #
    ######################################
    def submitOne(self, nn, tpn, caseFile):
	submit = TRUE
	submitResult = ""
        # open file and write the psub script
        if self.vals.machGroup == self.vals.default_machGroup:
            machine = "#NO-PSUB -c "
        else:
            machine = "#PSUB -c " + self.vals.machGroup + "\n"
        
        submitScript = caseFile + ".submit.tmp." + str(self.jobCount)
        fileScript = open(submitScript, 'w')
        fileScript.write("#!/bin/csh -v" + '\n')
        fileScript.write("#PSUB -s /bin/csh" + '\n')
        fileScript.write(machine)
        fileScript.write("#PSUB -eo" + '\n')
        fileScript.write("#PSUB -o " + caseFile + '\n')
        fileScript.write("#PSUB -tM " + str(self.vals.maxClockTime) + '\n')
        fileScript.write("#PSUB -ln " + str(nn) + '\n')
        fileScript.write("#PSUB -g " + str(int(nn) * int(tpn)) + '\n')
        fileScript.write("#PSUB -d " + str(self.vals.jobDepend) + '\n')
        if self.vals.startTime :
	    fileScript.write('#PSUB -A "' + str(self.vals.startTime) + '"\n')
        fileScript.write(" " + '\n')
        
        fileScript.write("cat " + self.caseTemp + " " + submitScript + '\n')
        fileScript.write("sleep 5 \n")
        fileScript.write('set echo \n')
        fileScript.write(self.vals.platformDict[self.vals.platform] + '\n')
        fileScript.write(' \n')
        fileScript.write(self.vals.jobStart + " " + self.vals.codeFile \
                         + " -f " + self.caseTemp + '\n')
        fileScript.write(" " + '\n')
	if self.debug == 1:
	    comment = "#"
        else:
	    comment = ""
        fileScript.write(comment + "rm -f " + submitScript + \
			 " " + self.caseTemp + '\n')
        fileScript.write("exit" + '\n')
	fileScript.close()

        # Now submit the job
        command = "psub"    
        command = command + " -x " + str(self.vals.psubOptions) \
            + " " + submitScript
        if self.debug == 1:
            print "\t** DEBUG **\n\t\t" + command + "\n\t** DEBUG **\n"
        else:
            try:
		childIn, childOut = os.popen4(command)
		childIn.close()
		returnSubmitString = childOut.read()
            except:
                showwarning(title="JOB NOT SUBMITTED",
			    message="unable to submit job, " + \
			    "check settings and retry")
		submit = FALSE
            else:
		try:
               	    self.vals.jobDepend = string.split(returnSubmitString)[1]
		    if not isInteger(self.vals.jobDepend):
			generalException = General()
			raise generalException
		except:
                    showwarning(title="JOB NOT SUBMITTED",
                                message="unable to submit job, " + \
				"check settings and retry")
        	    warningBox = Toplevel()
		    warningBox.title('ERROR DESCRIPTION')
		    textField = ScrolledText(warningBox)
		    textField.insert(At(0,0), returnSubmitString)
		    textField.pack(side=TOP)
		    textField.config(state=DISABLED)
		    Button(warningBox, text="OK",
			   command=warningBox.destroy).pack()
		    warningBox.focus_set()
		    warningBox.grab_set()
		    warningBox.wait_window()
		    submit = FALSE
		else:
		    self.strvar_jobDepend.set(str(self.vals.jobDepend))
		    submitResult = "\tpsub jobid = " + \
				   str(self.vals.jobDepend) + '\n'
	    childOut.close()
	return submit, submitResult


    ###########################################################
    # method to write parameter configurations to script file #
    ###########################################################
    def createScriptFile(self, nn, tpn, caseFile, preview):

	# preview option is used to view parameters before submitting
	if preview:
	    fileConfig = ''
	else:
            # open file and write
            # Parameters for case go in temp file before being
	    # copied to output file
            self.caseTemp = caseFile + ".tmp"
            fileConfig = open(self.caseTemp, 'w')
            fileConfig.write("IOR START" + '\n')

        blkList1 = toBytes(KIBIBYTE, self.vals.sel_KiB_BlockSizeList) + \
                   toBytes(MEBIBYTE, self.vals.sel_MiB_BlockSizeList)
        blkList2 = stringToList(self.vals.blockSize)
        blkList  = mergeLists(blkList1, blkList2)
        for blk in blkList:
            xferList1 = toBytes(KIBIBYTE, self.vals.sel_KiB_TransferSizeList) + \
                        toBytes(MEBIBYTE, self.vals.sel_MiB_TransferSizeList)
	    xferList2 = stringToList(self.vals.transferSize)
            xferList = mergeLists(xferList1, xferList2)
            for tsize in xferList:
		self.writeCaseParameters(nn, tpn, blk, tsize, fileConfig,
					 caseFile, preview)
		if not self.vals.moreReviews: break
	    if not self.vals.moreReviews: break
	if not preview:
            fileConfig.write("IOR STOP" + '\n')
            fileConfig.close()


    ###############################
    # method to display each test #
    ###############################
    def displayTest(self, test):

        ######################################
        # method to stop additional previews #
        ######################################
	def noMorePreviews():
	    showBox.destroy()
	    self.vals.moreReviews = FALSE

	showBox = Toplevel()
	showBox.title('IOR Job No. ' + str(self.jobCount) + \
		      ', Test No. ' + str(self.testCount))
    	textField = ScrolledText(showBox)
        textField.insert(At(0,0), test)
        textField.grid(row=0, column=0, columnspan=2, sticky=N)
        textField.config(state=DISABLED)
	Button(showBox, text="OK",
	       command=showBox.destroy).grid(row=1, column=0, sticky=E)
	Button(showBox, text="Cancel",
	       command=noMorePreviews).grid(row=1, column=1, sticky=W)
	showBox.focus_set()
	showBox.grab_set()
	showBox.wait_window()


    ###############################################
    # method to write one parameter configuration #
    ###############################################
    def writeCaseParameters(self, nn, tpn, blk, tsize, fileConfig,
			    caseFile, preview):
	parameters = [
 	    "API="		    	+ self.vals.sel_IOLayer,
	    "platform="		    	+ self.vals.platform,
	    "nodes="		    	+ str(nn),
	    "tasksPerNode="		+ str(tpn),
	    "blockSize="	    	+ str(blk),
	    "transferSize="	    	+ str(tsize),
	    "resultsFile="	    	+ caseFile,
	    "executable="	    	+ self.vals.codeFile,
	    "testFile="		    	+ self.vals.testFile,
	    "segmentCount="	    	+ str(self.vals.segmentCount),
	    "repetitions="	    	+ str(self.vals.testReps),
	    "numTasks="			+ str(self.vals.numTasks),
	    "interTestDelay="	    	+ str(self.vals.interTestDelay),
	    "setTimeStampSignature="	+ str(self.vals.setTimeStampSignature),
	    "maxTimeDuration="		+ str(self.vals.maxTimeDuration),
	    "outlierThreshold="		+ str(self.vals.outlierThreshold),
	    "filePerProc="	    	+ str(self.vals.filePerProc),
	    "keepFile="		    	+ str(self.vals.keepFile),
	    "keepFileWithError="    	+ str(self.vals.keepFileWithError),
	    "readFile="		    	+ str(self.vals.readFile),
	    "writeFile="	    	+ str(self.vals.writeFile),
	    "checkRead="	    	+ str(self.vals.checkRead),
	    "checkWrite="	    	+ str(self.vals.checkWrite),
	    "preallocate="	    	+ str(self.vals.preallocate),
	    "showHints="	    	+ str(self.vals.showHints),
	    "reorderTasks="	    	+ str(self.vals.reorderTasks),
	    "showHelp="	             	+ str(self.vals.showHelp),
	    "singleXferAttempt="	+ str(self.vals.singleXferAttempt),
	    "collective="	    	+ str(self.vals.collective),
	    "useFileView="	    	+ str(self.vals.useFileView),
	    "useSharedFilePointer="	+ str(self.vals.useSharedFilePointer),
	    "useStridedDatatype="   	+ str(self.vals.useStridedDatatype),
	    "useIndividualDataSets="	+ str(self.vals.useIndividualDataSets),
	    "useExistingTestFile="	+ str(self.vals.useExistingTestFile),
	    "noFill="			+ str(self.vals.noFill),
	    "intraTestBarriers="	+ str(self.vals.intraTestBarriers),
	    "storeFileOffset="		+ str(self.vals.storeFileOffset),
	    "multiFile="		+ str(self.vals.multiFile),
	    "quitOnError="		+ str(self.vals.quitOnError),
	    "fsync="			+ str(self.vals.fsync),
	    "uniqueDir="		+ str(self.vals.uniqueDir),
	    "useO_DIRECT="	        + str(self.vals.useO_DIRECT)
	]

	# special cases: C parser expects key=value
        if str(self.vals.hintFile) != "":
            parameters.append("hintFileName=" + str(self.vals.hintFile))
        if str(self.vals.options) != "":
            parameters.append("options=" + str(self.vals.options))

	# preview option is used to view parameters before submitting
	self.testCount = self.testCount + 1
	if preview:
	    test = ''
	    test = test + 'machine=' + self.vals.machGroup + '\n'
	    test = test + 'jobStart=' + self.vals.jobStart + '\n'
            test = test + 'maxClockTime=' + str(self.vals.maxClockTime) + '\n'
            test = test + 'startTime=' + str(self.vals.startTime) + '\n'
            test = test + 'psubOptions=' + str(self.vals.psubOptions) + '\n'
	    test = test + self.vals.platformDict[self.vals.platform]
	    for line in parameters:
		test = test + line + '\n'
	    self.displayTest(test)
	else:
	    for line in parameters:
		fileConfig.write('\t' + line + '\n')
	    fileConfig.write("RUN" + '\n')


    ########################################################
    # method to check entry boxes for numbers, not letters #
    ########################################################
    def validEntries(self):

        ###################################
        # method to show warning and exit #
        ###################################
        def warn(msg):
            showwarning(title="ERROR", message=msg)

        valid = TRUE

	# check nodes/tpn
        nodesToCheck = []
        tpnToCheck = []
        if not self.vals.nodes == "":
	    nodeList = stringToList(self.vals.nodes)
	    if nodeList == ERROR: nodeList = [ERROR]
            nodesToCheck = nodesToCheck + nodeList
        if not self.vals.tasksPerNode == "":
	    tpnList = stringToList(self.vals.tasksPerNode)
	    if tpnList == ERROR: tpnList = [ERROR]
            tpnToCheck = tpnToCheck + tpnList
        for x in nodesToCheck + tpnToCheck:
            if not isInteger(x) or int(str(x)) <= 0:
		warn("node and tpn count must be positive integers")
		valid = FALSE; break
	nodesToCheck = mergeLists(nodesToCheck, self.vals.sel_NodesList)
	tpnToCheck = mergeLists(tpnToCheck, self.vals.sel_TasksPerNodeList)

	# check block and transfer sizes
        blockEntriesToCheck = []
        transferEntriesToCheck = []
        if not self.vals.blockSize == "":
	    blockList = stringToList(self.vals.blockSize)
	    if blockList == ERROR: blockList = [ERROR]
            blockEntriesToCheck = blockEntriesToCheck + blockList
        if not self.vals.transferSize == "":
	    xferList = stringToList(self.vals.transferSize)
	    if xferList == ERROR: xferList = [ERROR]
            transferEntriesToCheck = transferEntriesToCheck + xferList
        for x in blockEntriesToCheck + transferEntriesToCheck:
	    # check block or transfer is acceptable value
	    if not isInteger(x) or \
	       long(str(x)) < long(0) or \
	       long(str(x)) % long(IOR_SIZE_T) != long(0):
		warning = "block and transfer entries must be nonnegative " + \
			  "integers, as well as multiple of access size (" + \
			  str(IOR_SIZE_T) + " bytes)"
		warn(warning)
		valid = FALSE; break

	    # check block or transfer < IOR_SIZE_T for MPIIO w/strided datatype
	    if long(str(x)) < long(IOR_SIZE_T) and \
	        self.vals.sel_IOLayer == 'MPIIO':
		warning = "block and transfer entries must be at least " + \
			  "access size (" + str(IOR_SIZE_T) + \
			  " bytes) for MPIIO"
		warn(warning)
		valid = FALSE; break

	    # check block or transfer < IOR_SIZE_T for HDF5
	    if long(str(x)) < long(IOR_SIZE_T) and \
                self.vals.sel_IOLayer == 'HDF5':
		warning = "block and transfer entries must be at least " + \
			  "access size (" + str(IOR_SIZE_T) + " bytes) for HDF5"
		warn(warning)
		valid = FALSE; break

	    # check block or transfer < IOR_SIZE_T for NCMPI
	    if long(str(x)) < long(IOR_SIZE_T) and \
                self.vals.sel_IOLayer == 'NCMPI':
		warning = "block and transfer entries must be at least " + \
			  "access size (" + str(IOR_SIZE_T) + \
			  " bytes) for NCMPI"
		warn(warning)
		valid = FALSE; break

	# check block size to be multiple of transfer size
	# also, check if transfer size is not larger than block size
	blkList = xferList = []
	blkList = toBytes(KIBIBYTE, self.vals.sel_KiB_BlockSizeList) + \
			  toBytes(MEBIBYTE, self.vals.sel_MiB_BlockSizeList)
	blkList = mergeLists(blkList, stringToList(self.vals.blockSize))
        xferList = toBytes(KIBIBYTE, self.vals.sel_KiB_TransferSizeList) + \
			   toBytes(MEBIBYTE, self.vals.sel_MiB_TransferSizeList)
	xferList = mergeLists(xferList, stringToList(self.vals.transferSize))
	blockEntriesToCheck = blockEntriesToCheck + blkList
	transferEntriesToCheck = transferEntriesToCheck + xferList
	for x in blockEntriesToCheck:
	    for y in transferEntriesToCheck:
		if long(str(x)) < long(str(y)):
		    warn("block size must not be smaller than transfer size")
		    valid = FALSE; break
		if long(str(y)) == long(0): y = '1'  # can't % 0
		if long(str(x)) % long(str(y)) != long(0):
		    warn("block size must be a multiple of transfer size")
		    valid = FALSE; break
	    if valid == FALSE:
		break

        # check if NCMPI file is greater than 2GiB
	if self.vals.sel_IOLayer == 'NCMPI':
	    for x in blockEntriesToCheck:
		for y in nodesToCheck:
		    for z in tpnToCheck:
			if long(long(str(x)) * long(str(y)) * long(str(z)) \
				* long(self.vals.segmentCount)) \
			    > long(2*GIBIBYTE):
			    warn("file size must be < 2GiB");
			    valid = FALSE; break
		    if valid == FALSE:
			break
		if valid == FALSE:
		    break

        # check if MPIIO strided datatype is greater than 2GiB
	if self.vals.useFileView and self.vals.sel_IOLayer == 'MPIIO':
	    for x in blockEntriesToCheck:
		for y in nodesToCheck:
		    for z in tpnToCheck:
			if long(long(str(x)) * long(str(y)) * long(str(z))) \
			    > long(2*GIBIBYTE):
			    warn("segment size must be < 2GiB");
			    valid = FALSE; break
		    if valid == FALSE:
			break
		if valid == FALSE:
		    break

        # can't check earliest start time easily; assume provided format
	# in entry box generally ensures coherence to acceptable values

	# check for max clock time
        if not isInteger(self.vals.maxClockTime) \
	   or int(str(self.vals.maxClockTime)) <= 0:
            warn("max clock time entry must be positive integer")
	    valid = FALSE

	# check for job dependency
        if not isInteger(self.vals.jobDepend) \
	   or int(str(self.vals.jobDepend)) < 0:
	    warn("job dependency entry must be nonnegative integer")
	    valid = FALSE

	# check for segment count
        if not isInteger(self.vals.segmentCount)  \
	   or int(str(self.vals.segmentCount)) <= 0:
	    warn("segment count must be positive integer")
	    valid = FALSE

	# check for test repetitions
        if not isInteger(self.vals.testReps) \
	   or int(str(self.vals.testReps)) <= 0:
	    warn("test repetition count must be positive integer")
	    valid = FALSE

	# check for number of tasks
        if not isInteger(self.vals.numTasks) \
	   or int(str(self.vals.numTasks)) < 0:
	    warn("number of tasks must be nonnegative integer")
	    valid = FALSE

	# check for intertest delay
        if not isInteger(self.vals.interTestDelay) \
	   or int(str(self.vals.interTestDelay)) < 0:
	    warn("intertest delay must be nonnegative integer")
	    valid = FALSE

	# check for verbosity
        if not isInteger(self.vals.verbose) \
	   or int(str(self.vals.verbose)) < 0:
	    warn("verbosity must be nonnegative integer")
	    valid = FALSE

	# check for time stamp signature
        if not isInteger(self.vals.setTimeStampSignature) \
	   or int(str(self.vals.setTimeStampSignature)) < 0:
	    warn("time stamp signature must be nonnegative integer")
	    valid = FALSE

	# check for max time duration
        if not isInteger(self.vals.maxTimeDuration) \
	   or int(str(self.vals.maxTimeDuration)) < 0:
	    warn("max test time must be nonnegative integer")
	    valid = FALSE

	# check for outlier threshold
        if not isInteger(self.vals.outlierThreshold) \
	   or int(str(self.vals.outlierThreshold)) < 0:
	    warn("outlier threshold must be nonnegative integer")
	    valid = FALSE

	# check for noFill
        if self.vals.noFill and self.vals.sel_IOLayer != 'HDF5':
	    warn("noFill only available in HDF5")
	    valid = FALSE

	# check for file-per-proc in HDF5
        if self.vals.filePerProc and self.vals.sel_IOLayer == 'HDF5':
	    warn("file-per-proc not available in current HDF5")
	    valid = FALSE

	# check for file-per-proc in NCMPI
        if self.vals.filePerProc and self.vals.sel_IOLayer == 'NCMPI':
	    warn("file-per-proc not available in current NCMPI")
	    valid = FALSE

	# check for preallocation
        if self.vals.preallocate and self.vals.sel_IOLayer != 'MPIIO':
	    warn("preallocation only available in MPIIO")
	    valid = FALSE

	# check for hints
        if self.vals.showHints and self.vals.sel_IOLayer == 'POSIX':
	    warn("hints not available in POSIX")
	    valid = FALSE

	# check for retry transfer
        if self.vals.singleXferAttempt and self.vals.sel_IOLayer != 'POSIX':
	    warn("retry transfer only available in POSIX")
	    valid = FALSE

	# check for collective
        if self.vals.collective and self.vals.sel_IOLayer == 'POSIX':
	    warn("collective not available in POSIX")
	    valid = FALSE

	# check for fileview
        if self.vals.useFileView and self.vals.sel_IOLayer != 'MPIIO':
	    warn("file view only available in MPIIO")
	    valid = FALSE

	""" may include this later if sufficient interest:
        if valid and self.vals.machGroup == self.vals.default_machGroup:
            valid = askokcancel(title="Submit without machine group",
                                message="No Machine Group was selected:\n" + \
                                "  OK to submit to any machine?\n"   \
                                "  or Cancel submit")
	"""
        return valid


    ############################################
    # method to collect values and submit jobs #
    ############################################
    def jobs(self, preview):
	noSubmit = FALSE
	allSubmitResults = "These jobs have been submitted:\n\n"
        self.jobCount = 0
	self.testCount = 0
        # Get all the selected values and put them into self.vals
        self.getSelectedValues()

	if self.validEntries():
            nodeList = mergeLists(self.vals.sel_NodesList,
				  stringToList(self.vals.nodes))
	    nodeList.sort()
            tasksPerNodeList = mergeLists(self.vals.sel_TasksPerNodeList,
                                          stringToList(self.vals.tasksPerNode))
	    tasksPerNodeList.sort()
	    jobsNum = len(nodeList) * len(tasksPerNodeList)

	    if preview:
		self.vals.moreReviews = TRUE
                for nn in nodeList:
                    for tpn in tasksPerNodeList:
                        self.jobCount = self.jobCount + 1
                        self.createScriptFile(nn, tpn, '', preview)
			if not self.vals.moreReviews: break
		    if not self.vals.moreReviews: break
	    else:
                for nn in nodeList:
                    for tpn in tasksPerNodeList:
                        self.jobCount = self.jobCount + 1
			now = time.localtime()
			elapsedSinceMidnight = now[3] * 60 * 60 + \
					       now[4] * 60 + \
					       now[5]
                        caseFile = self.vals.scriptFile + "." + \
				   str(self.vals.pid) + "-" + \
				   str(elapsedSinceMidnight) + \
				   str(self.jobCount);
                        self.createScriptFile(nn, tpn, caseFile, preview)
			submit, submitResult = self.submitOne(nn, tpn,
							       caseFile)
			allSubmitResults = allSubmitResults + submitResult
                        if (submit != TRUE):
			    noSubmit = TRUE; break
		    if noSubmit: break
	    if not noSubmit:
	        if not preview or self.vals.moreReviews:
		    if preview:
		        verbTense = ' to be submitted in '
		    else:
		        verbTense = ' submitted in '
			if self.testCount > 0:
			    showwarning(title="Jobs Submitted",
					message=allSubmitResults)
		    jobsNumBox = showinfo(title="Job Count",
				          message=str(self.testCount) + \
					  ' test' + "s"[self.testCount==1:] + \
					  verbTense +  str(jobsNum) + \
					  ' batch job' + "s"[jobsNum==1:])
        return


    ####################################
    # method to get name of scriptfile #
    ####################################
    def getScriptFileName(self):
        fileName = getFileName('IORscript', 'not-exist', self.vals.scriptDir,
                               extension='.IORscript')
        if fileName == "":
            return
        else:
            self.vals.scriptFile = fileName
            self.strvar_scriptFile.set(str(fileName))
            tmp = os.path.split(fileName)
	    # preserve chosen directory for next use
            self.vals.scriptDir = tmp[0]


    ################################################
    # method to get name of executable file - code #
    ################################################
    def getCodeFileName(self):
        fileName = getFileName('', 'exists', self.vals.binDir)
        if fileName == "":
            return
        else:
            self.vals.codeFile = fileName
            self.strvar_codeFile.set(str(fileName))
            tmp = os.path.split(fileName)
            self.vals.binDir = tmp[0]  # preserve chosen directory for next use


    ##################################
    # method to get name of testfile #
    ##################################
    def gettestFileName(self):
        fileName = getFileName('IORtemp', 'not-exist', self.vals.testFileDir,
                               extension='.IORTemp')
        if fileName == "":
            return
        else:
            self.vals.testFile = fileName
            self.strvar_testFile.set(str(fileName))
            tmp = os.path.split(fileName)
	    # preserve chosen directory for next use
            self.vals.testFileDir = tmp[0]


    ###################################
    # method to get name of hint file #
    ###################################
    def getHintFileName(self):
        fileName = getFileName('', 'exists', self.vals.hintFileDir)
        if fileName == "":
            return
        else:
            self.vals.hintFile = fileName
            self.strvar_hintFile.set(str(fileName))
            tmp = os.path.split(fileName)
	    # preserve chosen directory for next use
            self.vals.hintFileDir = tmp[0]


################################################################################
# main                                                                         #
################################################################################
root = Tk()
root.title('IOR Job Setup')
app = ConfigureWindow(root)
root.mainloop()
