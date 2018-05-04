from JetStreamDriver import *

class SunSpiderPayload(TestCase):
	TCMessageList = [
		TCMessage("3d-cube", ["3d-cube.js"]),
		TCMessage("3d-raytrace", ["3d-raytrace.js"]),
		TCMessage("base64", ["base64.js"]),
		TCMessage("crypto-aes", ["crypto-aes.js"]),
		TCMessage("crypto-md5", ["crypto-md5.js"]),
		TCMessage("crypto-sha1", ["crypto-sha1.js"]),
		TCMessage("date-format-tofte", ["date-format-tofte.js"]),
		TCMessage("date-format-xparb", ["date-format-xparb.js"]),
		TCMessage("n-body", ["n-body.js"]),
		TCMessage("regex-dna", ["regex-dna.js"]),
		TCMessage("tagcloud", ["tagcloud.js"]),
	]

	def addSunSpiderCPUWarmup(self, fd):
		fd.write('var warmupMS = 20;\n')
		fd.write('for (var start = new Date; new Date - start < warmupMS;) {\n')
		fd.write('if (Math.atan(Math.acos(Math.asin(Math.random()))) > 4) {\n')
		fd.write('console.log("Whoa, dude!");\n')
		fd.write('}\n')
		fd.write('}\n')

	def __init__(self, shellDir):
		super(SunSpiderPayload,self).__init__(shellDir,'SunSpiderPayload')
		for element in self.TCMessageList:
			element.benchmarks = []
			fileName = self.tmpFilePrefix + '/' + element.name + '.js'
			element.benchmarks.append(BenchMark(element.name, 'Latency'))
			element.benchmarks[0].results = []
			f = open(fileName, 'wr+')
			self.addSunSpiderCPUWarmup(f)
			self.addPrefix(f)
			self.addFiles(element.files,f)
			self.addSuffix(f,fileName)
			f.close()
			element.tmpFile = fileName

	def run(self):
		for element in self.TCMessageList:
			tmpFile = element.tmpFile
			sum = 0
			n = 20
			ret = 0
			failed = False
			for i in range(n):
				process = subprocess.Popen([self.shellDir, tmpFile], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
				stdout, stderr = process.communicate()
				if stdout.find('run_err') != -1:
					element.benchmarks[0].results.append(None)
					failed = True
					break
				else:
					ret = string.atof(stdout[stdout.find(tmpFile+':')+len(tmpFile)+1:].rstrip())
					sum += max(ret, 1)
			if failed == True:
				continue
			ret = sum/n
			element.benchmarks[0].results.append(100*self.reference[element.name]/ret)

	def reportResult(self):
		for element in self.TCMessageList:
			ret = TestCase.formatResult(element.benchmarks[0].results, False)
			print element.name + ': ' + ret
