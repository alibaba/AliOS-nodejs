from JetStreamDriver import *

class OctaneSuites(TestCase):

	TCMessageList = [
		TCMessage("code-multi-load", ["base.js", "code-load.js"])
	];

	def __init__(self, shellDir):
		super(OctaneSuites, self).__init__(shellDir, 'Octane')
		for element in self.TCMessageList:
			element.benchmarks = []
			fileName = self.tmpFilePrefix + '/' + element.name + '.js'
			element.benchmarks.append(BenchMark(element.name, 'Latency'))
			element.benchmarks[0].results = []
			f = open(fileName, 'wr+')
			self.addPrefix(f)
			self.addFiles(element.files,f)
			self.addRunCode(f)
			self.addSuffix(f,fileName)
			f.close()
			element.tmpFile = fileName

	def addRunCode(self, fd):
		fd.write('BenchmarkSuite.scores = [];\n')
		fd.write('var __suite = BenchmarkSuite.suites[0];\n')
		fd.write('for (var __thing = __suite.RunStep({}); __thing; __thing = __thing());\n')

	def addSuffix(self, fd, fileName):
		fd.write('print("'+fileName+':"+(BenchmarkSuite.GeometricMean(__suite.results)/1000))\n')
		#fd.write('}catch(err) {\n')
		#fd.write('print("run_err");\n')
		#fd.write('}\n')
		fd.flush()

	def addPrefix(self, fd):
		#fd.write('try {\n')
		pass

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
