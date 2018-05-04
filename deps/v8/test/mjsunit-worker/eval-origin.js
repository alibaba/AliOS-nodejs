// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --nostress-opt

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker//eval-origin.js");
	}
}
Error.prepareStackTrace = function(exception, frames) {
  return frames[0].getEvalOrigin();
}

var source = "new Error()";
var eval_origin;
var geval = eval;
var log = [];

(function() {
  log.push([geval(source).stack, "22:13"]);
  log.push([geval(source).stack, "23:13"]);
  // log.push([geval(source).stack, "19:13"]);  TODO(4921).
})();

(function() {
  log.push([eval(source).stack, "28:13"]);
  log.push([eval(source).stack, "29:13"]);
  // log.push([eval(source).stack, "25:13"]);  TODO(4921).
})();

log.push([eval(source).stack, "33:11"]);
log.push([eval(source).stack, "34:11"]);
// log.push([eval(source).stack, "30:11"]);  TODO(4921).

Error.prepareStackTrace = undefined;

for (var item of log) {
  var stacktraceline = item[0];
  var expectation = item[1];
  var re = new RegExp(`:${expectation}\\)$`);
  assertTrue(re.test(stacktraceline));
}
