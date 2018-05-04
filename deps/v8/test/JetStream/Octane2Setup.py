from JetStreamDriver import *
import re

class Octane2Suites(TestCase):
	TCMessageList = [
		TCMessage("richards", ["richards.js"]),
		TCMessage("delta-blue", ["deltablue.js"]),
		TCMessage("crypto", ["crypto.js"]),
		TCMessage("proto-raytracer", ["raytrace.js"]),
		TCMessage("earley-boyer", ["earley-boyer.js"]),
		TCMessage("regexp-2010", ["regexp.js"]),
		TCMessage("splay", ["splay.js"], TCMessage.ALL),
		TCMessage("navier-stokes", ["navier-stokes.js"]),
		TCMessage("pdfjs", ["pdfjs.js"]),
		TCMessage("mandreel", ["mandreel.js"], TCMessage.ALL),
		TCMessage("gbemu", ["gbemu-part1.js", "gbemu-part2.js"]),
		TCMessage("code-first-load", ["code-load.js"], TCMessage.LATENCY),
		TCMessage("box2d", ["box2d.js"]),
		TCMessage("zlib", ["zlib.js", "zlib-data.js"]),
		TCMessage("typescript", ["typescript.js", "typescript-input.js", "typescript-compiler.js"], TCMessage.LATENCY)
	];

	def addRunCode(self, fd):
		fd.write('BenchmarkSuite.scores = [];\n')
		fd.write('var __suite = BenchmarkSuite.suites[0];\n')
		fd.write('for (var __thing = __suite.RunStep({}); __thing; __thing = __thing());\n')

	def addPrefix(self, fd):
		#fd.write('try {\n')
		pass

	def addSuffix(self, fd, fileName):
		fd.write('print("'+fileName+':"+(BenchmarkSuite.GeometricMeanTime(__suite.results)/1000)+","+(BenchmarkSuite.GeometricMeanLatency(__suite.results)/1000))\n')
		#fd.write('}catch(err) {\n')
		#fd.write('print("run_err");\n')
		#fd.write('}\n')
		fd.flush()

	def __init__(self, shellDir):
		super(Octane2Suites, self).__init__(shellDir, 'Octane2')
		for element in self.TCMessageList:
			element.benchmarks = []
			fileName = self.tmpFilePrefix + '/' + element.name + '.js'
			element.benchmarks.append(BenchMark(element.name))
			element.benchmarks[0].results = []
			if element.latency == TCMessage.ALL:
				element.benchmarks.append(BenchMark(element.name+'-latency', 'Latency'))
				element.benchmarks[1].results = []
			elif element.latency == TCMessage.LATENCY:
				element.benchmarks[0].latency = 'Latency'

			element.files.insert(0,"base.js")
			f = open(fileName, 'wr+')
			self.addPrefix(f)
			self.addFiles(element.files,f)
			self.addRunCode(f)
			self.addSuffix(f,fileName)
			f.close()
			element.tmpFile = fileName

	def run(self):
		for element in self.TCMessageList:
			tmpFile = element.tmpFile
			process = subprocess.Popen([self.shellDir, tmpFile], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
			stdout, stderr = process.communicate()
			source = re.compile(r'[0-9.]*')
			if stdout.find('run_err') != -1:
				for benchmark in element.benchmarks:
					benchmark.results.append(None)
			else:
				retidx = stdout.find(tmpFile+':')+len(tmpFile)+1
				for benchmark in element.benchmarks:
					seg = stdout[retidx:].find(',')
					benchmark.results.append(100*self.reference[benchmark.name]/string.atof(source.search(stdout[retidx:]).group()))
					retidx += seg + 1

	def reportResult(self):
		for element in self.TCMessageList:
			for benchmark in element.benchmarks:
				ret = TestCase.formatResult(benchmark.results, False)
				print benchmark.name + ': ' + ret
