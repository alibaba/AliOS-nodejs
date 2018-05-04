# -*- coding: utf-8 -*- 
from JetStreamDriver import *
from SimpleSetup import SimplePayload
from SunSpiderSetup import SunSpiderPayload
from Octane2Setup import Octane2Suites
from OctaneSetup import OctaneSuites
from CDjsSetup import CDjsSuites
import time
import sys
import string

def run(shellDir, num):
	testcases = [OctaneSuites(shellDir), CDjsSuites(shellDir), Octane2Suites(shellDir),SimplePayload(shellDir), SunSpiderPayload(shellDir)]
	categoryMaps = {}
	categoryNames = []
	benchmarks = []
	for testcase in testcases:
		for TCMessage in testcase.TCMessageList:
			benchmarks.extend(TCMessage.benchmarks)
			for benchmark in TCMessage.benchmarks:
				categoryMaps[benchmark.category] = []
	for key in categoryMaps:
		categoryNames.append(key)
	for i in range(num):
		for testcase in testcases:
			testcase.run()
	for testcase in testcases:
		testcase.reportResult()
		testcase.rmTempDirAndFiles()
	TestCase.reportGeomeans(benchmarks, categoryNames)

def Usege():
	print 'python run.py <d8 dir> [num]'
	print '      num:    loop Count'


if __name__ == '__main__':
	argc = len(sys.argv)
	shellDir = ''
	num = 3

	if argc < 2:
		print '请输入参数: d8路径'
		Usege()
		sys.exit(1)
	elif argc == 3:
		shellDir = sys.argv[1]
		num = string.atoi(sys.argv[2])
	elif argc == 2:
		shellDir = sys.argv[1]
	else:
		print '参数太多'
		Usege()
		sys.exit(2)
	if shellDir == '' or num <= 0:
		print '参数错误'
		print 'shellDir is %s, num is %d' % (shellDir, num)
		sys.exit(3)
	run(shellDir, num)
