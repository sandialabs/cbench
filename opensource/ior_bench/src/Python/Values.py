#/*****************************************************************************\
#*                                                                             *
#*       Copyright (c) 2003, The Regents of the University of California       *
#*     See the file COPYRIGHT for a complete copyright notice and license.     *
#*                                                                             *
#\*****************************************************************************/
#
# CVS info:
#   $RCSfile: Values.py,v $
#   $Revision: 1.55 $
#   $Date: 2006/07/12 00:04:47 $
#   $Author: loewe $

from math import *
from Tkinter import *
from tkMessageBox import *                                # sets ERROR = 'error'
import os.path
import pickle
from definitions import *

################################################################################
# class to hold default values for configuration file                          #
################################################################################
class Values:
    ############################
    # initialization of values #
    ############################
    def __init__(self):

    # help file
	self.helpFile = "README"
    # I/O layer
	self.IOLayer = ["POSIX", "MPIIO", "HDF5", "NCMPI"]
	self.sel_IOLayer = "POSIX"

    # platform parameters
	self.default_platform = "<none>"
	self.platform = ""

	# IBM Platform Parameters 
	environSettings = [
	    "setenv MP_RMPOOL 1",
	    "setenv MP_INFOLEVEL 1",
	    "setenv MP_EUILIB us",
	    "setenv MP_LABELIO no",
	    "setenv MP_SHARED_MEMORY yes",
	    "setenv MP_HINTS_FILTERED no",
	    "setenv IOR_HINT__MPI__IBM_largeblock_io true"
	    #
	    # MP_HINTS_FILTERED affects the behavior of MPI_Info routines.
	    # If set to YES, which is the default, MPI_Info routines will
	    # ignore hint keys which are not understood by the implemen-
	    # tation.  Arbitrary key-value pairs cannot be placed in and
	    # retrieved from info objects.  Setting this to NO allows
	    # arbitrary key-value pairs to be cached in an info object,
	    # with no indication of whether the implementation understands
	    # them.
	    #
	    # IOR_HINTs are the form:
	    #     IOR_HINT__[GPFS|MPI]__the_hint_itself
	    #
	]
	self.AIXplatformText = ""
	for environ in environSettings:
	    self.AIXplatformText = self.AIXplatformText + environ + "\n"
	self.sel_platformText = self.AIXplatformText

	self.platformDict = { '<none>' : '', \
			      'AIX'    : self.AIXplatformText, \
			      'IRIX'   : '', \
			      'LINUX'  : '', \
			      'TRU64'  : '', \
			      'OTHER'  : '' }
	# Changes here must also be made in createPlatformParameters

        # jobStart parameters
	self.default_jobStart = "<none>"
	self.jobStart = ""

	self.jobStarts =   ["poe",
			   "prun",
                           "srun",
			   "mpirun"]
	# Changes here must also be made in createJobStartParameters

	# submit parameters
	self.startTime = os.popen("date +%D\' \'%H:%M:%S").read()[:-1]
	self.default_machGroup = "<none>"
	self.machGroup = ""
	self.machGroups = ["adelie",
			   "blue",
			   "frost",
			   "gps",
                           "mcr",
			   "snow",
			   "tc2k",
			   "white"]
	self.maxClockTime = 120
	self.jobDepend = 0
	self.psubOptions = ""

	# method to return list of powers of two from start to end
	# ex: PowersOfTwo(4, 32) == [4, 8, 16, 32]
	def PowersOfTwo(start, end):
	    start = int(log(start)/log(2))
	    end = int((log(end)/log(2)) + 1)
	    return map(lambda x: 2**x, range(start, end))


    # system resources
	self.nodes = ""
	self.nodesList = PowersOfTwo(1, 512)
	self.sel_NodesList = []
	self.tasksPerNode = ""
	self.tasksPerNodeList = PowersOfTwo(1, 32)
	self.sel_TasksPerNodeList = []

    # file locations
	pwd = os.popen("cd ../..; pwd").read()[:-1]
	self.scriptFile = pwd + "/iorscript"
	self.codeFile = pwd + "/src/C/IOR"
	self.testFile = pwd + "/iorTestFile"
	self.hintFile = ""

    # test parameters
	self.testReps = 3
	self.numTasks = 0
	self.interTestDelay = 0
	self.segmentCount = 1
	self.transferSize = ""
	self.KiB_transferSizeList = PowersOfTwo(1, 512)
	self.sel_KiB_TransferSizeList = []
	self.MiB_transferSizeList = PowersOfTwo(1, 4096)
	self.sel_MiB_TransferSizeList = []
	self.blockSize = ""
	self.KiB_blockSizeList = PowersOfTwo(1, 512)
	self.sel_KiB_BlockSizeList = []
	self.MiB_blockSizeList = PowersOfTwo(1, 4096)
	self.sel_MiB_BlockSizeList = []

	# additional options
	self.filePerProc = 0
	self.keepFile = 0
	self.keepFileWithError = 0
	self.readFile = 1
	self.writeFile = 1
	self.checkRead = 0
	self.checkWrite = 0
	self.collective = 0
	self.preallocate = 0
	self.useFileView = 0
	self.useSharedFilePointer = 0
	self.useStridedDatatype = 0
	self.useIndividualDataSets = 0
	self.storeFileOffset = 0
	self.multiFile = 0
	self.useExistingTestFile = 0
	self.fsync = 0
	self.noFill = 0
	self.quitOnError = 0
	self.useO_DIRECT = 0
	self.verbose = 0
	self.options = ""
	self.showHints = 0
	self.reorderTasks = 0
	self.showHelp = 0
	self.singleXferAttempt = 0
	self.intraTestBarriers = 0
	self.uniqueDir = 0
	self.maxTimeDuration = 0
	self.outlierThreshold = WC_OL_THRESHOLD
	self.setTimeStampSignature = 0

    # internal GUI parameters
	self.testCount = 0
	self.jobCount = 0
	self.moreReviews = TRUE
	self.pid = os.getpid()
	self.localDir = os.getcwd()
	self.scriptDir = self.localDir
	self.binDir = self.localDir
	self.testFileDir = self.localDir
	self.hintFileDir = self.localDir
	self.saveConfigDir = self.localDir
	self.loadConfigDir = self.localDir


    #####################################
    # method to load settings from file #
    #####################################
    def loadSettingsFromFile(self, fileName):
	try:
    	    openFile = open(fileName, 'r')
	    self = pickle.load(openFile)
	    openFile.close()
	except:
	    showwarning(title="ERROR", message="unable to load file")
	    self = ERROR
	return self


    ###################################
    # method to save settings to file #
    ###################################
    def saveSettingsToFile(self, fileName):
	try:
	    openFile = open(fileName, 'w')
	    pickle.dump(self, openFile)
	    openFile.close()
	except:
	    showwarning(title="ERROR", message="unable to save file")
