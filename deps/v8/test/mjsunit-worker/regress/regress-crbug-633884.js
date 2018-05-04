// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-crbug-633884.js");
	}
}
try {
  // Leave "blarg" as the hole in a new ScriptContext.
  Realm.eval(Realm.current(), "throw Error(); let blarg");
} catch (e) { }

// Access "blarg" via a dynamic lookup. Should not crash!
assertThrows(function() {
  // Prevent full-codegen from optimizing away the %LoadLookupSlot call.
  eval("var x = 5");
  blarg;
});
