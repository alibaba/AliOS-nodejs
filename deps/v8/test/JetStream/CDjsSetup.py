from JetStreamDriver import *

class CDjsSuites(TestCase):

	CDjsFiles = [
		"constants.js",
		"util.js",
		"red_black_tree.js",
		"call_sign.js",
		"vector_2d.js",
		"vector_3d.js",
		"motion.js",
		"reduce_collision_set.js",
		"simulator.js",
		"collision.js",
		"collision_detector.js",
		"benchmark.js"
	];
	TCMessageList = {TCMessage('cdjs', CDjsFiles, TCMessage.LATENCY)}

	def addPrefix(self, fd):
		fd.write('try {\n')

	def addSuffix(self, fd, fileName):
		fd.write('print("'+fileName+':"+benchmark());\n')
		fd.write('}catch(err) {\n')
		fd.write('print("run_err");\n')
		fd.write('}\n')


	def __init__(self, shellDir):
		super(CDjsSuites, self).__init__(shellDir, 'CDjs')
		for element in self.TCMessageList:
			element.benchmarks = []
			fileName = self.tmpFilePrefix + '/' + element.name + '.js'
			element.benchmarks.append(BenchMark(element.name, 'Latency'))
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
