import shutil
import subprocess
import sys
import os
import string
import math


class BenchMark:
	name = ""
	category = ""
	results = []
	def __init__(self, name, category="Throughput"):
		self.name = name
		self.category = category


class TCMessage:
	name = ""
	files = []
	latency = 0
	benchmarks = []
	tmpFile = ""
	LATENCY = 1
	ALL = 2

	def __init__(self, name, files, latency=0):
		self.name = name
		self.files = files
		self.latency = latency

class Result:
	num = 0
	mean = 0
	interval = 0
	failed = 0
	def __init__(self, num=0,mean=0,interval=0):
		self.num = num
		self.mean = mean
		self.interval = interval

	def setFailed(self, failed):
		self.failed = failed

class TestCase(object):
	tDistribution = [None, None, 12.71, 4.30, 3.18, 2.78, 2.57, 2.45, 2.36, 2.31, 2.26, 2.23, 2.20, 2.18, 2.16, 2.14, 2.13, 2.12, 2.11, 2.10, 2.09, 2.09, 2.08, 2.07, 2.07, 2.06, 2.06, 2.06, 2.05, 2.05, 2.05, 2.04, 2.04, 2.04, 2.03, 2.03, 2.03, 2.03, 2.03, 2.02, 2.02, 2.02, 2.02, 2.02, 2.02, 2.02, 2.01, 2.01, 2.01, 2.01, 2.01, 2.01, 2.01, 2.01, 2.01, 2.00, 2.00, 2.00, 2.00, 2.00, 2.00, 2.00, 2.00, 2.00, 2.00, 2.00, 2.00, 2.00, 2.00, 2.00, 1.99, 1.99, 1.99, 1.99, 1.99, 1.99, 1.99, 1.99, 1.99, 1.99, 1.99, 1.99, 1.99, 1.99, 1.99, 1.99, 1.99, 1.99, 1.99, 1.99, 1.99, 1.99, 1.99, 1.99, 1.99, 1.99, 1.99, 1.98, 1.98, 1.98, 1.98, 1.98, 1.98, 1.98, 1.98, 1.98, 1.98, 1.98, 1.98, 1.98, 1.98, 1.98, 1.98, 1.98, 1.98, 1.98, 1.98, 1.98, 1.98, 1.98, 1.98, 1.98, 1.98, 1.98, 1.98, 1.98, 1.98, 1.98, 1.98, 1.98, 1.98, 1.98, 1.98, 1.98, 1.98, 1.98, 1.98, 1.98, 1.98, 1.98, 1.98, 1.98, 1.98, 1.98, 1.98, 1.98, 1.98, 1.98, 1.98, 1.98, 1.98, 1.98, 1.98, 1.98, 1.98, 1.98, 1.98, 1.98, 1.98, 1.98, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.97, 1.96]
	tMax = len(tDistribution)
	tLimit = 1.96
	reference = {
	    "3d-cube": 7.3,
	    "3d-raytrace": 8.05,
	    "base64": 4.2,
	    "crypto-aes": 6.6,
	    "crypto-md5": 3,
	    "crypto-sha1": 2.05,
	    "date-format-tofte": 9.95,
	    "date-format-xparb": 8.1,
	    "fannkuch-redux": 6,
	    "n-body": 3,
	    "regex-dna": 7.85,
	    "tagcloud": 14.55,
	    "unpack-code": 27.65,
	    "bigfib.cpp": 1731,
	    "c-ray.c": 950,
	    "container.cpp": 10981,
	    "dry.c": 1699,
	    "float-mm.c": 2669,
	    "gcc-loops.cpp": 18038,
	    "hash-map": 565,
	    "n-body.c": 2444,
	    "quicksort.c": 1238,
	    "towers.c": 778,
	    "code-multi-load": 2.120195905897577,
	    "richards": 0.23239600278875197,
	    "delta-blue": 0.46860356138706644,
	    "crypto": 1.5442204655506482,
	    "proto-raytracer": 3.134796238244515,
	    "earley-boyer": 2.5795868328750444,
	    "regexp-2010": 61.82352941176467,
	    "splay": 0.9286376274328075,
	    "splay-latency": 40, 
	    "navier-stokes": 9.653846153846146,
	    "pdfjs": 88.4166666666666,
	    "mandreel": 157.14285714285708,
	    "mandreel-latency": 10, 
	    "gbemu": 135.9999999999998,
	    "code-first-load": 2.3249465349251905,
	    "box2d": 28.416666666666636,
	    "zlib": 887.666666666666,
	    "typescript": 1149.9999999999993,
	    "lua": 29858,
	    "cdjs": 20, 
	    "geomean": 31.556451704472156,
	}

	modeName = ''
	tmpFilePrefix = ''
	shellDir = ''

	@staticmethod
	def tDist(n):
		if (n > TestCase.tMax):
			return TestCase.tLimit
		return TestCase.tDistribution[n]

	def __init__(self,shellDir,modeName):
		self.shellDir = shellDir
		self.modeName = modeName
		if modeName == None or modeName == '':
			raise NameError('modeName is illegal')
		self.tmpFilePrefix = '/tmp/' + modeName
		if os.path.exists(self.tmpFilePrefix) == True:
			shutil.rmtree(self.tmpFilePrefix)
		os.mkdir(self.tmpFilePrefix)

	def run(self):
		pass

	def addPrefix(self,fd):
		fd.write('try {\n')
		fd.write('var startTime = new Date();\n')

	def addSuffix(self,fd, fileName):
		fd.write('print("'+fileName+':"+ (new Date()-startTime));\n')
		fd.write('}catch(err) {\n')
		fd.write('print("run_err");\n')
		fd.write('}\n')
		fd.flush()

	def addFiles(self,files,fd):
		for file in files:
			orgName = os.path.join(sys.path[0], self.modeName, file)
			org = open(orgName, 'r')
			fd.writelines(org.readlines())
			org.close()

	def rmTempDirAndFiles(self):
		shutil.rmtree(self.tmpFilePrefix)

	@staticmethod
	def computeStatistics(values):
		length = len(values)
		if (length == 0):
			return Result()

		sum = 0
		n = 0
		for i in range(length):
			if (values[i] == None):
				continue
			sum += values[i]
			n+=1

		if (n != length):
			ret = Result(length)
			ret.setFailed(length - n)
			return ret

		mean = sum / n

		if (n <= 2):
			return Result(n, mean)

		sumForStdDev = 0
		for i in range(length):
			if (values[i] == None):
				continue
			sumForStdDev += math.pow(values[i] - mean, 2)

		standardDeviation = math.sqrt(sumForStdDev / (n - 1))
		standardError = standardDeviation / math.sqrt(n)
		interval = TestCase.tDist(n) * standardError
		return Result(n, mean, interval)

	@staticmethod
	def formatResult(values, extra):
		extraPrecision = 1 if extra else 0

		def prepare(value):
			precision = 4 + extraPrecision;
			digitsAfter = 0
			if (value):
				log = math.log(value) / math.log(10);
				if (log >= precision):
					digitsAfter = 0;
				elif (log < 0):
					digitsAfter = precision
				else:
					digitsAfter = precision - 1 - int(log)
			else:
				digitsAfter = precision - 1

			return round(value, digitsAfter)

		statistics = TestCase.computeStatistics(values)

		if (statistics.num == 0):
			return ''

		if (statistics.failed != 0):
			if (statistics.num == 1):
				return 'ERROR'
			return 'ERROR (failed ' + str(statistics.failed) + '/' + str(statistics.num) + ')'

		if (statistics.interval != 0):
			return str(prepare(statistics.mean)) + ' +/-' + str(prepare(statistics.interval))

		return str(prepare(statistics.mean))

	@staticmethod
	def allSelector(benchmark):
		return True

	@staticmethod
	def createCategorySelector(category):
		return lambda(benchmark): benchmark.category == category

	@staticmethod
	def computeGeomeans(selector, benchmarks):
		geomeans = []
		idx = 0
		while True:
			sum = 0
			numDone = 0
			numSelected = 0
			allfinished = True
			for i in range(len(benchmarks)):
				if selector(benchmarks[i]) == False:
					continue
				numSelected += 1
				if idx >= len(benchmarks[i].results):
					allfinished = False
					break
				if benchmarks[i].results[idx] == None:
					continue
				sum += math.log(benchmarks[i].results[idx])
				numDone += 1
			if allfinished == False:
				break
			if numDone != numSelected:
				geomeans.append(None)
			else:
				geomeans.append(math.exp(sum / numDone))
			idx += 1
		return geomeans

	@staticmethod
	def reportGeomeans(benchmarks, categoryNames):
		for categoryName in categoryNames:
			selector = TestCase.createCategorySelector(categoryName)
			print categoryName + ': ' + TestCase.formatResult(TestCase.computeGeomeans(selector, benchmarks), True)
		print 'Score: ' + TestCase.formatResult(TestCase.computeGeomeans(TestCase.allSelector, benchmarks), True)
