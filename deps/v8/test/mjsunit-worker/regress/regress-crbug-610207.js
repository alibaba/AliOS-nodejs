// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-crbug-610207.js");
	}
}
Error.prepareStackTrace = function(exception, frames) {
  return frames[0].getEvalOrigin();
}

try {
  Realm.eval(0, "throw new Error('boom');");
} catch(e) {
  print(e.stack);
}
