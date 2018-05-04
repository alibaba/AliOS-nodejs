// Copyright 2015 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax --use-osr

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-crbug-522895.js");
	}
}
var body =
  "function bar1(  )  {" +
  "  var i = 35;       " +
  "  while (i-- > 31) {" +
  "    %OptimizeOsr(); " +
  "    j = 9;          " +
  "    while (j-- > 7);" +
  "  }                 " +
  "  return i;         " +
  "}";

function gen() {
  return eval("(" + body + ")");
}

gen()();
