import shutil
import sys, os, stat, re
ignore_file = ["mjsunit.js"]
def iterator_file():
    global ignore_file;
    path = "test/mjsunit-worker/"
    for dirpath,dirnames,filenames in os.walk(path):
        for file in filenames:
            if file in ignore_file or file.split(".")[-1] != "js":
                continue;
            else:
                yield dirpath + "/" + file

def gen_worker_js():
    for source_file in iterator_file():
       dst_path = source_file + ".worker"
       shutil.copyfile(source_file, dst_path);

def gen_worker_call():
    append_str = "if (!isworker()) {\n\tfor (var i = 0; i < ThreadWorkerCount; i++) {\n";
    append_str += "\tvar worker = new ThreadWorker(\"test/mjsunit-worker/mjsunit.js\",\"";
    for source_file in iterator_file():
           f = open(source_file, "r");
           filelines = f.readlines();
           output_lines = []
           jump_over = True;
           append_worker = False;
           need_use_strict = False;
           jump_over_use_strict = True;
           ##jump over #
           for line in filelines:
               if jump_over and (line.startswith("//") or line == "\n"):
                   output_lines.append(line);
                   continue;
               elif jump_over_use_strict and (line.startswith("\"use strict\"")):
                   jump_over_use_strict = False;
                   need_use_strict = True;
               else:
                   jump_over = False;
               if not append_worker:
                   current_append_worker = append_str + source_file + "\");\n\t}\n}\n"
                   output_lines.append(current_append_worker)
                   append_worker = True;
               output_lines.append(line)

           f.close();
           f = open(source_file, "w");
           if need_use_strict:
               f.writelines("\"use strict\"\n");
           f.writelines(output_lines);


#gen_worker_js();
gen_worker_call();
