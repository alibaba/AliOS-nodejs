#!/usr/bin/python
import sys

def Parser(lines, map):
  testcase = ''
  isError = False
  for line in lines:
    if line == '\n':
      continue
    line = line.lstrip(' ')
    line = line.strip('\n')
    if line == '':
      continue
    if line[0] == '[':
      testcase = line[line.rfind(' ')+1:]
      map[testcase] = ['pass']
      isError = False
    elif isError == True:
      map[testcase] = map[testcase] + [line]
    elif line[:4] == '=== ':
      isError = True
      map[testcase] = ['failed', line]

def printLog(log):
  for line in log:
    print line

NO_CASE = 'noCase'
def Main():
  try:
    if len(sys.argv) != 3:
      print "python compare.py <result1> <result2>"
      return
    file1 = open(sys.argv[1], "rU+")
    file2 = open(sys.argv[2], "rU+")
    file1_lines = file1.readlines()
    file2_lines = file2.readlines()
    file1_map = {}
    file2_map = {}
    Parser(file1_lines, file1_map)
    Parser(file2_lines, file2_map)
    print('{0:70s} {1:8s} {2:8s}'.format('testCaseName', 'result1', 'result2'))
    errorNum1 = 0
    errorNum2 = 0
    noCase1 = 0
    noCase2 = 0
    for k,v in file1_map.iteritems():
      if not file2_map.has_key(k):
        print('{0:70s} {1:8s} {2:8s}'.format(k, v[0], NO_CASE))
        noCase2 += 1
        continue
      if file2_map[k][0] != v[0]:
        print('{0:70s} {1:8s} {2:8s}'.format(k, v[0], file2_map[k][0]))
        if v[0] == 'pass':
          errorNum2 += 1
          printLog(file2_map[k][1:])
        else:
          errorNum1 += 1
          printLog(v[1:])
      del file2_map[k]

    for k,v in file2_map.iteritems():
      print('{0:70s} {1:8s} {2:8s}'.format(k, NO_CASE, v[0]))
      noCase1 += 1

    if errorNum1 != 0 or errorNum2 != 0:
      print('{0:70s} {1:8s} {2:8s}'.format('errorNums', str(errorNum1), str(errorNum2)))
    if noCase1 != 0 or noCase2 != 0:
      print('{0:70s} {1:8s} {2:8s}'.format('noCaseNums', str(noCase1), str(noCase2)))
    if noCase1 == 0 and noCase2 == 0 and errorNum1 == 0 and errorNum2 == 0:
      print 'The test result are same'
  except IOError as err:
    print('File Error:' + str(err))

if __name__ == "__main__":
  sys.exit(Main())
