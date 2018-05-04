#-*- coding:utf-8 -*-
# author:   liyong.zly
# file:     analysis_ram_usage.py
# created:  Thu Feb 23 12:45:03 CST 2017
# modified: Thu Feb 23 12:45:03 CST 2017
import os, sys
import optparse
import json
import pandas as pd
import matplotlib.pyplot as plt
from hp2jsoncsv import ConvertLogFileToJson

SAMPLE_DATA_LABEL = 'SampleData'
SAMPLE_TIME_LABEL = 'SampleTime'
KEYS= ['RSS', 'PSS', 'SharedDirty', 'SharedClean', 'PrivateDirty', 'PrivateClean']

def GenKeyName(filename, command_line):
  return filename.replace("\"", "").replace(" ", "_")

def Parse(file):
  json_data = ConvertLogFileToJson(file)
  json_data['FileName'] = file
  return json_data

def CompareOneKeyData(raw_data, key):
  key_data_dict = {}
  for data in raw_data:
    sample_data = data[SAMPLE_DATA_LABEL]
    # print(sample_data[0][key])
    key_data = [(item[SAMPLE_TIME_LABEL], item[key]) for item in sample_data ]
    key_name = GenKeyName(data['FileName'], data['CommandLine'])
    key_data_dict[key_name] = key_data
  return key_data_dict

def GetFigureName(key, subkey):
  return key.replace("\"", "").replace(" ", "_")

def OutputCompareResult(result_data_dict, keys, options):
  for key in keys:
    series_list = list()
    subkey_list = list()
    data = result_data_dict[key]
    #print(key)
    df_data_dict = {}
    sample_time_ruler = []
    sample_internal = 0
    for subkey in data.keys():
      subkey_of_data = data[subkey]
      sample_internal = int(subkey_of_data[1][0]) - int(subkey_of_data[0][0])
      series = pd.Series([value[1] for value in subkey_of_data],
                         index = [(int(item[0]) / sample_internal) for item in subkey_of_data],
                         dtype = int)
      series._update_inplace(series.sort_index())
      df_data_dict[subkey] = series
      subkey_list.append(subkey)

    data_frame = pd.DataFrame.from_dict(df_data_dict, orient='index')
    data_frame_filled = data_frame.T.fillna(0)
    if options.output_csv_file:
      data_frame_filled.to_csv(GetFigureName(key, subkey) +'.csv')
    plt.figure()
    data_frame_filled.plot()
    plt.legend(loc='best')
    plt.ylabel(key+' Unit(kB)')
    plt.xlabel('Sample Time(in ' + str(sample_internal) + 'microsecond)')
    if options.show_figure:
      plt.show(block=True)

    if options.output_pdf_file:
      fig_name = GetFigureName(key, subkey) + '.pdf'
      plt.savefig(fig_name, bbox_inches='tight')


def AnalysisRamUsage(args, options):
  ram_usage_data_list = list()
  compare_keys = []
  for argfile in args:
    ram_usage_data_list.append(Parse(argfile))

  if options.compare_rss:
    compare_keys.append('RSS')
  if options.compare_pss:
    compare_keys.append('PSS')
  if options.compare_shared_clean:
    compare_keys.append('SharedClean')
  if options.compare_shared_dirty:
    compare_keys.append('SharedDirty')
  if options.compare_private_clean:
    compare_keys.append('PrivateClean')
  if options.compare_private_dirty:
    compare_keys.append('PrivateDirty')

  # TODO check keys in compare_keys are all legal keys

  compare_result = {}
  for key in compare_keys:
    data = CompareOneKeyData(ram_usage_data_list, key)
    compare_result[key] = data

  OutputCompareResult(compare_result, compare_keys, options)


def BuildOptions():
  result = optparse.OptionParser()
  result.add_option("--compare-rss", default=False,
                    help="Compare RSS(Resident Set Size)", action="store_true")
  result.add_option("--compare-pss", default=False,
                    help="Compare PSS(Proportional Set Size))", action="store_true")
  result.add_option("--compare-shared-dirty", default=False,
                    help="Compare Shared Dirty memory", action="store_true")
  result.add_option("--compare-shared-clean", default=False,
                    help="Compare Shared Clean memory", action="store_true")
  result.add_option("--compare-private-dirty", default=False,
                    help="Compare Private Dirty memory", action="store_true")
  result.add_option("--compare-private-clean", default=False,
                    help="Compare Private Clean memory", action="store_true")
  result.add_option("--show-figure", default=False,
                    help="Show figure on the screen, require user is using GUI", action="store_true")
  result.add_option("--output-pdf-file", default=True,
                    help="Save comparation result as pdf file format", action="store_true")
  result.add_option("--output-csv-file", default=False,
                    help="Save comparation result as csv file format", action="store_true")
  return result

def ProcessOptions(options):
  if not options.compare_rss and not options.compare_pss \
    and not options.compare_shared_dirty and not options.compare_shared_clean \
    and not options.compare_private_dirty and not options.compare_private_clean:
    return False
  return True


def Main():
  parser = BuildOptions()
  (options, args) = parser.parse_args()
  if not ProcessOptions(options):
    parser.print_help()
    sys.exit();

  if not args:
    sys.exit('Missing logfile');
  if len(args) < 2:
    sys.exit('Required providing two or more than two data file generated by show_ram_usage.sh');

  AnalysisRamUsage(args, options)

if __name__ == '__main__':
  sys.exit(Main())
