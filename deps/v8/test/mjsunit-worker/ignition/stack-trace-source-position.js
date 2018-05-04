// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --no-turbo

// TODO(yangguo): fix for turbofan

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/ignition/stack-trace-source-position.js");
	}
}
function f(x) {
  if (x == 0) {
    return new Error().stack;
  }
  return f(x - 1);
}

var stack_lines = f(2).split("\n");

assertTrue(/at f \(.*?:16:12\)/.test(stack_lines[1]));
assertTrue(/at f \(.*?:18:10\)/.test(stack_lines[2]));
assertTrue(/at f \(.*?:18:10\)/.test(stack_lines[3]));
