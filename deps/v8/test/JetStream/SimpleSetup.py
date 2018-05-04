from JetStreamDriver import *


class SimplePayload(TestCase):
	TCMessageList = [
		TCMessage("bigfib.cpp", ["bigfib.cpp.js"]),
		TCMessage("container.cpp", ["container.cpp.js"]),
		TCMessage("dry.c", ["dry.c.js"]),
		TCMessage("float-mm.c", ["float-mm.c.js"]),
		TCMessage("gcc-loops.cpp", ["gcc-loops.cpp.js"]),
		TCMessage("hash-map", ["hash-map.js"]),
		TCMessage("n-body.c", ["n-body.c.js"]),
		TCMessage("quicksort.c", ["quicksort.c.js"]),
		TCMessage("towers.c", ["towers.c.js"])
	]
	
	def __init__(self, shellDir):
		super(SimplePayload, self).__init__(shellDir, 'SimplePayload')
		for element in self.TCMessageList:
			element.benchmarks = []
			fileName = self.tmpFilePrefix + '/' + element.name + '.js'
			element.benchmarks.append(BenchMark(element.name))
			element.benchmarks[0].results = []
			f = open(fileName, 'wr+')
			self.addPrefix(f)
			self.addFiles(element.files,f)
			self.addSuffix(f,fileName)
			f.close()
			element.tmpFile = fileName
		
	def run(self):
		for element in self.TCMessageList:
			tmpFile = element.tmpFile
			process = subprocess.Popen([self.shellDir, tmpFile], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
			stdout, stderr = process.communicate()
			if stdout.find('run_err') != -1:
				element.benchmarks[0].results.append(None)
			else:
				ret = string.atof(stdout[stdout.find(tmpFile+':')+len(tmpFile)+1:].rstrip())
				element.benchmarks[0].results.append(100*self.reference[element.name]/ret)

	def reportResult(self):
		for element in self.TCMessageList:
			ret = TestCase.formatResult(element.benchmarks[0].results, False)
			print element.name + ': ' + ret
