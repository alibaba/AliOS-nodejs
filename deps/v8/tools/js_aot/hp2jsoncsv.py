#!/env/python
# author:   liyong.zly
# file:     hp2json.python
# created:  Wed Feb 22 10:47:39 CST 2017
# modified: Wed Feb 22 10:47:39 CST 2017
import os, sys, traceback
import json
import csv, optparse
import codecs

def ConvertLogFileToJson(filename):
  json_result = {}
  sample_data = list()
  command_line = 'Command: '
  test_date = 'Date: '
  sample_unit = 'SampleUnit: '
  value_unit = 'ValueUnit: '
  sample_time = 'SampleTime:'
  sampling = False
  try:
    logfile = open(filename, 'r')
    try:
      for line in logfile.readlines():
        text_line = ' '.join( x.strip() for x in line.split()[1:])
        if line.startswith('JOB'):
          command_line = text_line
          continue

        if line.startswith('DATE'):
          test_date = text_line
          continue

        if line.startswith('SAMPLE_UNIT'):
          sample_unit = text_line
          continue

        if line.startswith('VALUE_UNIT'):
          value_unit = text_line
          continue

        if line.startswith('BEGIN_SAMPLE'):
          sample_time = text_line
          sampling= True
          continue
        if line.startswith('END_SAMPLE'):
          end_sample_time = text_line
          if (sample_time != end_sample_time):
            print('ERROR: begin sample time:' + sample_time + \
                  ' is not same as end sample time: ' + end_sample_time)
            sys.exit()

          #print("Append one sample data")
          sample_data.append({'SampleTime': sample_time, 'RSS': rss, 'PSS': pss,\
                  'SharedClean': shared_clean, 'SharedDirty': shared_ditry,
                  'PrivateClean': private_clean, 'PrivateDirty': private_dirty})
          sampling = False
          continue
        if sampling:
          text_line = line.split()[1].strip()
          if line.startswith('RSS:'):
            rss = text_line
            continue
          if line.startswith('PSS:'):
            pss = text_line
            continue
          if line.startswith('SharedClean:'):
            shared_clean = text_line
            continue
          if line.startswith('SharedDirty:'):
            shared_ditry = text_line
            continue
          if line.startswith('PrivateClean:'):
            private_clean =  text_line
            continue
          if line.startswith('PrivateDirty:'):
            private_dirty = text_line
            continue
          else:
            print('Ignore data: ' + line)

    finally:
      logfile.close()
      json_result = {'CommandLine': command_line, 'TestDate': test_date,\
              'SampleUnit': sample_unit, 'ValueUnit': value_unit, \
              'SampleData': sample_data}
      return json_result

  except Exception, e:
    print("Exception:" + str(e))
    print(traceback.format_exc())
    print('can\'t open %s' % filename)
    return None


def ProcessLogFile(filename, options):
  json_result = ConvertLogFileToJson(filename)
  if json_result == None:
    sys.exit('Failed to parse log file %s' %filename)

  if options.json_file:
    json_file_name = options.json_file
    if os.path.exists(json_file_name) and os.path.is_file(json_file_name):
      os.remove(json_file_name)
    reload(sys)
    sys.setdefaultencoding('utf-8')
    json_name = codecs.open(json_file_name, 'w', 'utf-8')

    result_json = json.dumps(json_result)
    json_name.write(result_json)
    json_name.write('\n')
    json_name.close()

  if options.csv_file:
    csv_file_name = options.csv_file
    if os.path.exists(csv_file_name) and os.path.is_file(csv_file_name):
      os.remove(csv_file_name)
    csv_file = open(csv_file_name, 'w')
    output = csv.writer(csv_file)
    #output.writerow(json_result['SampleData'][0].keys())  # header row
    header = ['SampleTime', 'RSS', 'PSS', 'PrivateClean', 'PrivateDirty', 'SharedClean', 'SharedDirty']
    output.writerow(header)
    for row in json_result['SampleData']:
      output.writerow([row['SampleTime'], row['RSS'], row['PSS'], row['PrivateClean'], \
                    row['PrivateDirty'], row['SharedClean'], row['SharedDirty']])
    command_line_str = ['CommandLine', json_result['CommandLine'].replace("\"","")]
    output.writerow(command_line_str)
    test_date_str = ['TestDate', json_result['TestDate'].replace("\"","")]
    output.writerow(test_date_str)
    sample_unit_str = ['SampleUnit', json_result['SampleUnit'].replace("\"","")]
    output.writerow(sample_unit_str)
    value_unit_str = ['ValueUnit', json_result['ValueUnit'].replace("\"","")]
    output.writerow(value_unit_str)
    csv_file.close()


def BuildOptions():
  result = optparse.OptionParser()
  result.add_option("--json-file", dest="json_file",
                    help="Output json file name", action="store")
  result.add_option("--csv-file", dest="csv_file",
                    help="Output Csv file name", action="store")
  return result

def ProcessOptions(options):
  if not options.json_file and not options.csv_file:
    return False
  return True


def Main():
  parser = BuildOptions()
  (options, args) = parser.parse_args()
  if not ProcessOptions(options):
    parser.print_help()
    sys.exit();

  if not args:
    print "Missing logfile"
    sys.exit();

  ProcessLogFile(args[0], options)

if __name__ == '__main__':
  sys.exit(Main())
